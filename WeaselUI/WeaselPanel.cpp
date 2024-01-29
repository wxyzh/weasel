#include "pch.h"
#include "DirectWriteResources.h"
#include "WeaselPanel.h"
#include <WeaselCommon.h>
#include "VersionHelpers.hpp"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "FullScreenLayout.h"
#include "VHorizontalLayout.h"
#include <xkeycheck.h>
// for IDI_ZH, IDI_EN
#include <resource.h>
#include "test.h"
#define USE_MOUSE_EVENTS
#define COLORTRANSPARENT(color)		((color & 0xff000000) == 0)
#define COLORNOTTRANSPARENT(color)	((color & 0xff000000) != 0)
#define TRANS_COLOR		0x00000000
#define GDPCOLOR_FROM_COLORREF(color)	Gdiplus::Color::MakeARGB(((color >> 24) & 0xff), GetRValue(color), GetGValue(color), GetBValue(color))
#define IS_FULLSCREENLAYOUT(style)	(style.layout_type == UIStyle::LAYOUT_VERTICAL_FULLSCREEN || style.layout_type == UIStyle::LAYOUT_HORIZONTAL_FULLSCREEN)
#define NOT_FULLSCREENLAYOUT(style)	(style.layout_type != UIStyle::LAYOUT_VERTICAL_FULLSCREEN && style.layout_type != UIStyle::LAYOUT_HORIZONTAL_FULLSCREEN)

#pragma comment(lib, "Shcore.lib")
import Utility;

inline void LoadIconNecessary(std::wstring& a, std::wstring& b, WTL::CIcon& c, int d) {
	if (a == b) return;
	a = b;
	if (b.empty())
	{
		c.LoadIconW(d, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	}
	else
	{
		c = (HICON)LoadImage(NULL, b.c_str(), IMAGE_ICON, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_LOADFROMFILE);
	}
}

static inline void ReconfigRoundInfo(IsToRoundStruct& rd, const int& i, const int& m_candidateCount)
{
	if (i == 0 && m_candidateCount > 1) {
		std::swap(rd.IsTopLeftNeedToRound, rd.IsBottomLeftNeedToRound);
		std::swap(rd.IsTopRightNeedToRound, rd.IsBottomRightNeedToRound);
	}
	if (i == m_candidateCount - 1) {
		std::swap(rd.IsTopLeftNeedToRound, rd.IsBottomLeftNeedToRound);
		std::swap(rd.IsTopRightNeedToRound, rd.IsBottomRightNeedToRound);
	}
}

WeaselPanel::WeaselPanel(weasel::UI& ui) :
	m_layout(NULL),
	m_ctx(ui.ctx()),
	m_octx{ ui.octx() },
	m_status(ui.status()),
	m_style(ui.style()),
	m_ostyle(ui.ostyle()),
	m_candidateCount(0),
	m_current_zhung_icon(),
	dpi(96),
	hide_candidates(false),
	m_pDWR{ ui.pdwr() },
	_m_gdiplusToken(0),
	_UICallback{ ui.uiCallback() },
	_SetRectCallback{ ui.SetRectCallback() },
	m_following{ ui.GetCaretFollowing() }
{
	m_iconDisabled.LoadIconW(IDI_RELOAD, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	m_iconEnabled.LoadIconW(IDI_ZH, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	m_iconAlpha.LoadIconW(IDI_EN, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	m_iconFull.LoadIconW(IDI_FULL_SHAPE, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	m_iconHalf.LoadIconW(IDI_HALF_SHAPE, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	// for gdi+ drawings, initialization
	GdiplusStartup(&_m_gdiplusToken, &_m_gdiplusStartupInput, NULL);

	_InitFontRes();
	m_ostyle = m_style;
}

WeaselPanel::~WeaselPanel()
{
	Gdiplus::GdiplusShutdown(_m_gdiplusToken);
	delete m_layout;
	m_layout = nullptr;
}

void WeaselPanel::_ResizeWindow()
{
	CDCHandle dc = GetDC();
	CSize m_size = m_layout->GetContentSize();
	SetWindowPos(NULL, 0, 0, m_size.cx, m_size.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
	ReleaseDC(dc);
}

void WeaselPanel::_CreateLayout()
{
	if (m_layout != NULL)
		delete m_layout;

	Layout* layout = NULL;
	if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
	{
		layout = new VHorizontalLayout(m_style, m_ctx, m_status);
	}
	else
	{
		if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL || m_style.layout_type == UIStyle::LAYOUT_VERTICAL_FULLSCREEN)
		{
			layout = new VerticalLayout(m_style, m_ctx, m_status);
		}
		else if (m_style.layout_type == UIStyle::LAYOUT_HORIZONTAL || m_style.layout_type == UIStyle::LAYOUT_HORIZONTAL_FULLSCREEN)
		{
			layout = new HorizontalLayout(m_style, m_ctx, m_status);
		}

		if (IS_FULLSCREENLAYOUT(m_style))
		{
			layout = new FullScreenLayout(m_style, m_ctx, m_status, m_inputPos, layout);
		}
	}
	m_layout = layout;
}

//更新界面
void WeaselPanel::Refresh()
{
	bool should_show_icon = (m_status.ascii_mode || !m_status.composing || !m_ctx.aux.empty());
	m_candidateCount = (BYTE)m_ctx.cinfo.candies.size();
	// check if to hide candidates window
	// show tips status, two kind of situation: 1) only aux strings, don't care icon status; 2)only icon(ascii mode switching)
	bool show_tips = (!m_ctx.aux.empty() && m_ctx.cinfo.empty() && m_ctx.preedit.empty()) || (m_ctx.empty() && should_show_icon);
	// show schema menu status: schema_id == L".default"
	bool show_schema_menu = m_status.schema_id == L".default";
	bool margin_negative = (m_style.margin_x < 0 || m_style.margin_y < 0);
	// when to hide_cadidates?
	// 1. margin_negative, and not in show tips mode( ascii switching / half-full switching / simp-trad switching / error tips), and not in schema menu
	hide_candidates = margin_negative && !show_tips && !show_schema_menu;

	// only RedrawWindow if no need to hide candidates window
	if (!hide_candidates)
	{
		_InitFontRes();
		_CreateLayout();

		CDCHandle dc = GetDC();
		m_layout->DoLayout(dc, m_pDWR);
		ReleaseDC(dc);
		_ResizeWindow();
		_RepositionWindow();

		if (m_ctx != m_octx)
		{
			m_octx = m_ctx;
			RedrawWindow();
		}
	}
}

void WeaselPanel::_InitFontRes(bool forced)
{
	HMONITOR hMonitor = MonitorFromRect(m_inputPos, MONITOR_DEFAULTTONEAREST);
	UINT dpiX = 96, dpiY = 96;
	if (hMonitor)
		GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
	// prepare d2d1 resources
	// if style changed, or dpi changed, or pDWR nullptr, re-initialize directwrite resources
	if (forced || m_pDWR == nullptr || m_ostyle != m_style || dpiX != dpi)
	{
		m_pDWR.reset();
		m_pDWR = std::make_shared<DirectWriteResources>(m_style, dpiX);
		m_pDWR->pRenderTarget->SetTextAntialiasMode((D2D1_TEXT_ANTIALIAS_MODE)m_style.antialias_mode);
	}
	m_ostyle = m_style;
	dpi = dpiX;
}

static HBITMAP CopyDCToBitmap(HDC hDC, LPRECT lpRect)
{
	if (!hDC || !lpRect || IsRectEmpty(lpRect)) return NULL;
	HDC hMemDC;
	HBITMAP hBitmap, hOldBitmap;
	int nX, nY, nX2, nY2;
	int nWidth, nHeight;

	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;

	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hDC, nWidth, nHeight);
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hDC, nX, nY, nWidth, nHeight, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

	DeleteDC(hMemDC);
	DeleteObject(hOldBitmap);
	return hBitmap;
}

void WeaselPanel::_CaptureRect(CRect& rect)
{
	HDC ScreenDC = ::GetDC(NULL);
	CRect rc;
	GetWindowRect(&rc);
	POINT WindowPosAtScreen = { rc.left, rc.top };
	rect.OffsetRect(WindowPosAtScreen);
	// capture input window
	if (OpenClipboard()) {
		HBITMAP bmp = CopyDCToBitmap(ScreenDC, LPRECT(rect));
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, bmp);
		CloseClipboard();
		DeleteObject(bmp);
	}
	ReleaseDC(ScreenDC);
}

LRESULT WeaselPanel::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = true;
	return MA_NOACTIVATE;
}

// simulating a key down and up
static void SendInputKey(WORD key)
{
	std::array<INPUT, 2> inputs;
	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki = { key, 0,0,0,0 };
	inputs[1].type = INPUT_KEYBOARD;
	inputs[1].ki = { key, 0,KEYEVENTF_KEYUP,0,0 };
	::SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
}

LRESULT WeaselPanel::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int delta = GET_WHEEL_DELTA_WPARAM(wParam);
	if (_UICallback && delta != 0)
	{
		bool nextpage = delta < 0;
		_UICallback(nullptr, nullptr, &nextpage);
	}
	bHandled = true;
	return 0;
}

LRESULT WeaselPanel::OnLeftClicked(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (hide_candidates)
	{
		bHandled = true;
		return 0;
	}
	CPoint point;
	point.x = GET_X_LPARAM(lParam);
	point.y = GET_Y_LPARAM(lParam);

	if (!m_following)
	{
		RECT rc{};
		GetClientRect(&rc);
		if (abs(point.y - rc.top) < 10 || abs(point.y - rc.bottom) < 10 || abs(point.x - rc.left) < 10 || abs(point.x - rc.right) < 10)
		{
			PostMessage(WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
			m_holder = true;
			SetCapture();
		}
	}

	// capture
	if (m_style.click_to_capture)
	{
		CRect recth = m_layout->GetCandidateRect((int)m_ctx.cinfo.highlighted);
		if (m_istorepos)	recth.OffsetRect(0, m_offsetys[m_ctx.cinfo.highlighted]);
		recth.InflateRect(m_style.hilite_padding_x, m_style.hilite_padding_y);
		// capture widow
		if (recth.PtInRect(point)) _CaptureRect(recth);
		else
		{
			// if shadow_color transparent, decrease the capture rectangle size
			if (COLORTRANSPARENT(m_style.shadow_color) && m_style.shadow_radius != 0)
			{
				CRect crc{ rcw };
				int shadow_gap = (m_style.shadow_offset_x == 0 && m_style.shadow_offset_y == 0)
					? 2 * m_style.shadow_radius
					: m_style.shadow_radius + m_style.shadow_radius / 2;
				int ofx = m_style.hilite_padding_x + abs(m_style.shadow_offset_x) + shadow_gap > abs(m_style.margin_x)
					? m_style.hilite_padding_x + abs(m_style.shadow_offset_x) + shadow_gap - abs(m_style.margin_x) : 0;
				int ofy = m_style.hilite_padding_y + abs(m_style.shadow_offset_y) + shadow_gap > abs(m_style.margin_y)
					? m_style.hilite_padding_y + abs(m_style.shadow_offset_y) + shadow_gap - abs(m_style.margin_y) : 0;
				crc.DeflateRect(m_layout->offsetX - ofx, m_layout->offsetY - ofy);
				_CaptureRect(crc);
			}
			else
			{
				_CaptureRect(rcw);
			}
		}
	}
	// button response
	{
		if (!m_style.inline_preedit && m_candidateCount != 0 && COLORNOTTRANSPARENT(m_style.prevpage_color) && COLORNOTTRANSPARENT(m_style.nextpage_color)) {
			// click prepage
			if (m_ctx.cinfo.currentPage != 0) {
				CRect prc = m_layout->GetPrepageRect();
				if (m_istorepos)	prc.OffsetRect(0, m_offsety_preedit);
				if (prc.PtInRect(point)) {
					bool nextPage = false;
					if (_UICallback)
						_UICallback(nullptr, nullptr, &nextPage);
					bHandled = true;
					return 0;
				}
			}
			// click nextpage
			if (!m_ctx.cinfo.is_last_page) {
				CRect prc = m_layout->GetNextpageRect();
				if (m_istorepos)	prc.OffsetRect(0, m_offsety_preedit);
				if (prc.PtInRect(point)) {
					bool nextPage = true;
					if (_UICallback)
						_UICallback(nullptr, nullptr, &nextPage);
					bHandled = true;
					return 0;
				}
			}
		}
		// select by click
		for (auto i = 0; i < m_candidateCount && i < MAX_CANDIDATES_COUNT; ++i) {
			CRect rect = m_layout->GetCandidateRect((int)i);
			if (m_istorepos)	rect.OffsetRect(0, m_offsetys[i]);
			rect.InflateRect(m_style.hilite_padding_x, m_style.hilite_padding_y);
			if (rect.PtInRect(point))
			{
				if (_UICallback)
				{
					_UICallback(&i, nullptr, nullptr);
				}
				break;
			}
		}
	}
	bHandled = true;
	return 0;
}

LRESULT WeaselPanel::OnLeftReleased(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (!m_following && m_holder)
	{
		RECT rcWindow;
		GetWindowRect(&rcWindow);
		if (m_reversed)
		{
			rcWindow.top = rcWindow.bottom;
		}
		else
		{
			rcWindow.bottom = rcWindow.top;
		}
		rcWindow.right = rcWindow.left;
		_SetRectCallback(rcWindow);

		m_holder = false;
		ReleaseCapture();
	}
	bHandled = true;
	return 0;
}

LRESULT WeaselPanel::OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPoint point;
	point.x = GET_X_LPARAM(lParam);
	point.y = GET_Y_LPARAM(lParam);

	if (!m_following)
	{
		RECT rc{};
		GetClientRect(&rc);
		if (abs(point.y - rc.top) < 10 || abs(point.y - rc.bottom) < 10 || abs(point.x - rc.left) < 10 || abs(point.x - rc.right) < 10)
		{
			SetCursor(LoadCursor(nullptr, IDC_SIZEALL));			
		}
		else
		{
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		}
	}

	for (int i = 0; i < m_candidateCount && i < MAX_CANDIDATES_COUNT; ++i) {
		CRect rect = m_layout->GetCandidateRect((int)i);

		if (m_istorepos)
			rect.OffsetRect(0, m_offsetys[i]);

		if (rect.PtInRect(point))
		{
			if (_UICallback && i != m_ctx.cinfo.highlighted && m_style.mouse_hover_ms)
				_UICallback(nullptr, &i, nullptr);

			SetCursor(LoadCursor(nullptr, IDC_HAND));
		}
	}
	bHandled = true;
	return 0;
}

LRESULT WeaselPanel::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_mouse_entry == false)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = m_style.mouse_hover_ms ? m_style.mouse_hover_ms : 10;	// uint: ms
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	if (!m_following)
	{
		static CPoint ptPreview{};

		CPoint point;
		point.x = GET_X_LPARAM(lParam);
		point.y = GET_Y_LPARAM(lParam);
		if (m_holder && point != ptPreview)
		{
			CPoint ptTemp{ point - ptPreview };
			CRect rc{};

			GetWindowRect(&rc);
			rc.OffsetRect(ptTemp);
			MoveWindow(&rc);
			return 0;
		}
		ptPreview = point;
	}
	return 0;
}

LRESULT WeaselPanel::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_mouse_entry = false;
	return 0;
}

void WeaselPanel::_HighlightText(CDCHandle& dc, const CRect& rc, const COLORREF& color, const COLORREF& shadowColor, int radius, const BackType& type, const IsToRoundStruct& rd, const COLORREF& bordercolor)
{
	// Graphics obj with SmoothingMode
	Gdiplus::Graphics g_back(dc);
	g_back.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

	// blur buffer
	int blurMarginX = m_layout->offsetX;
	int blurMarginY = m_layout->offsetY;

	GraphicsRoundRectPath* hiliteBackPath;
	if (rd.Hemispherical && type != BackType::BACKGROUND && NOT_FULLSCREENLAYOUT(m_style))
		hiliteBackPath = new GraphicsRoundRectPath(rc, m_style.round_corner_ex - (m_style.border % 2 ? m_style.border / 2 : 0), rd.IsTopLeftNeedToRound, rd.IsTopRightNeedToRound, rd.IsBottomRightNeedToRound, rd.IsBottomLeftNeedToRound);
	else // background or current candidate background not out of window background
		hiliteBackPath = new GraphicsRoundRectPath(rc, radius);

	// 必须shadow_color都是非完全透明色才做绘制, 全屏状态不绘制阴影保证响应速度
	if (m_style.shadow_radius && COLORNOTTRANSPARENT(shadowColor) && NOT_FULLSCREENLAYOUT(m_style)) {
		CRect rect(
			blurMarginX + m_style.shadow_offset_x,
			blurMarginY + m_style.shadow_offset_y,
			rc.Width() + blurMarginX + m_style.shadow_offset_x,
			rc.Height() + blurMarginY + m_style.shadow_offset_y);
		BYTE r = GetRValue(shadowColor);
		BYTE g = GetGValue(shadowColor);
		BYTE b = GetBValue(shadowColor);
		BYTE alpha = (BYTE)(shadowColor >> 24);
		Gdiplus::Color shadow_color = Gdiplus::Color::MakeARGB(alpha, r, g, b);
		static Gdiplus::Bitmap* pBitmapDropShadow;
		pBitmapDropShadow = new Gdiplus::Bitmap((INT)rc.Width() + blurMarginX * 2, (INT)rc.Height() + blurMarginY * 2, PixelFormat32bppPARGB);

		Gdiplus::Graphics g_shadow(pBitmapDropShadow);
		g_shadow.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		// dropshadow, draw a roundrectangle to blur
		if (m_style.shadow_offset_x != 0 || m_style.shadow_offset_y != 0) {
			GraphicsRoundRectPath shadow_path(rect, radius);
			Gdiplus::SolidBrush shadow_brush(shadow_color);
			g_shadow.FillPath(&shadow_brush, &shadow_path);
		}
		// round shadow, draw multilines as base round line
		else {
			int step = alpha / m_style.shadow_radius / 2;
			Gdiplus::Pen pen_shadow(shadow_color, (Gdiplus::REAL)1);
			for (int i = 0; i < m_style.shadow_radius; i++) {
				GraphicsRoundRectPath round_path(rect, radius + 1 + i);
				g_shadow.DrawPath(&pen_shadow, &round_path);
				shadow_color = Gdiplus::Color::MakeARGB(alpha - i * step, r, g, b);
				pen_shadow.SetColor(shadow_color);
				rect.InflateRect(1, 1);
			}
		}
		DoGaussianBlur(pBitmapDropShadow, (float)m_style.shadow_radius, (float)m_style.shadow_radius);

		g_back.DrawImage(pBitmapDropShadow, INT(rc.left - blurMarginX), INT(rc.top - blurMarginY));

		// free memory
		delete pBitmapDropShadow;
		pBitmapDropShadow = NULL;
	}

	// 必须back_color非完全透明才绘制
	if (COLORNOTTRANSPARENT(color)) {
		Gdiplus::Color back_color = GDPCOLOR_FROM_COLORREF(color);
		Gdiplus::SolidBrush back_brush(back_color);
		g_back.FillPath(&back_brush, hiliteBackPath);
	}
	// draw border, for bordercolor not transparent and border valid
	if (COLORNOTTRANSPARENT(bordercolor) && m_style.border > 0)
	{
		Gdiplus::Color border_color = GDPCOLOR_FROM_COLORREF(bordercolor);
		Gdiplus::Pen gPenBorder(border_color, (Gdiplus::REAL)m_style.border);
		// candidate window border
		if (type == BackType::BACKGROUND) {
			GraphicsRoundRectPath bgPath(rc, m_style.round_corner_ex);
			g_back.DrawPath(&gPenBorder, &bgPath);
		}
		else if (type != BackType::TEXT)	// hilited_candidate_border / candidate_border
			g_back.DrawPath(&gPenBorder, hiliteBackPath);
	}
	// free memory
	delete hiliteBackPath;
	hiliteBackPath = NULL;
}

// draw preedit text, text only
bool WeaselPanel::_DrawPreedit(Text const& text, CDCHandle dc, CRect const& rc)
{
	bool drawn = false;
	std::wstring const& t = text.str;
	auto txtFormat = m_pDWR->pPreeditTextFormat;

	if (!t.empty()) {
		weasel::TextRange range;
		std::vector<weasel::TextAttribute> const& attrs = text.attributes;
		for (size_t j = 0; j < attrs.size(); ++j)
			if (attrs[j].type == weasel::HIGHLIGHTED)
				range = attrs[j].range;

		if (range.start < range.end) {
			CSize beforeSz, hilitedSz, afterSz;
			std::wstring before_str = t.substr(0, range.start);
			std::wstring hilited_str = t.substr(range.start, range.end);
			std::wstring after_str = t.substr(range.end);
			m_layout->GetTextSizeDW(before_str, before_str.length(), txtFormat.Get(), m_pDWR, &beforeSz);
			m_layout->GetTextSizeDW(hilited_str, hilited_str.length(), txtFormat.Get(), m_pDWR, &hilitedSz);
			m_layout->GetTextSizeDW(after_str, after_str.length(), txtFormat.Get(), m_pDWR, &afterSz);

			int x = rc.left;
			int y = rc.top;

			if (range.start > 0) {
				// zzz
				std::wstring str_before(t.substr(0, range.start));
				CRect rc_before;
				if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
					rc_before = CRect(rc.left, y, rc.right, y + beforeSz.cy);
				else
					rc_before = CRect(x, rc.top, rc.left + beforeSz.cx, rc.bottom);
				_TextOut(rc_before, str_before, str_before.length(), m_style.text_color, txtFormat.Get());
				if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
					y += beforeSz.cy + m_style.hilite_spacing;
				else
					x += beforeSz.cx + m_style.hilite_spacing;
			}
			{
				// zzz[yyy]
				std::wstring str_highlight(t.substr(range.start, range.end - range.start));
				CRect rc_hi;

				if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
					rc_hi = CRect(rc.left, y, rc.right, y + hilitedSz.cy);
				else
					rc_hi = CRect(x, rc.top, x + hilitedSz.cx, rc.bottom);
				_TextOut(rc_hi, str_highlight, str_highlight.length(), m_style.hilited_text_color, txtFormat.Get());
				if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
					y += rc_hi.Height() + m_style.hilite_spacing;
				else
					x += rc_hi.Width() + m_style.hilite_spacing;
			}
			if (range.end < static_cast<int>(t.length())) {
				// zzz[yyy]xxx
				std::wstring str_after(t.substr(range.end));
				CRect rc_after;
				if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
					rc_after = CRect(rc.left, y, rc.right, y + afterSz.cy);
				else
					rc_after = CRect(x, rc.top, x + afterSz.cx, rc.bottom);
				_TextOut(rc_after, str_after, str_after.length(), m_style.text_color, txtFormat.Get());
			}
		}
		else {
			CRect rcText(rc.left, rc.top, rc.right, rc.bottom);
			_TextOut(rcText, t, t.length(), m_style.text_color, txtFormat.Get());
		}
		// draw pager mark if not inline_preedit if necessary
		if (m_candidateCount && !m_style.inline_preedit && COLORNOTTRANSPARENT(m_style.prevpage_color) && COLORNOTTRANSPARENT(m_style.nextpage_color))
		{
			const std::wstring pre = L"<";
			const std::wstring next = L">";
			CRect prc = m_layout->GetPrepageRect();
			// clickable color / disabled color
			int color = m_ctx.cinfo.currentPage ? m_style.prevpage_color : m_style.text_color;
			if (m_istorepos) prc.OffsetRect(0, m_offsety_preedit);
			_TextOut(prc, pre, pre.length(), color, txtFormat.Get());

			CRect nrc = m_layout->GetNextpageRect();
			// clickable color / disabled color
			color = m_ctx.cinfo.is_last_page ? m_style.text_color : m_style.nextpage_color;
			if (m_istorepos) nrc.OffsetRect(0, m_offsety_preedit);
			_TextOut(nrc, next, next.length(), color, txtFormat.Get());
		}
		drawn = true;
	}
	return drawn;
}

// draw hilited back color, back only
bool WeaselPanel::_DrawPreeditBack(Text const& text, CDCHandle dc, CRect const& rc)
{
	bool drawn = false;
	std::wstring const& t = text.str;
	IDWriteTextFormat1* txtFormat = m_pDWR->pPreeditTextFormat.Get();

	if (!t.empty()) {
		weasel::TextRange range;
		std::vector<weasel::TextAttribute> const& attrs = text.attributes;
		for (size_t j = 0; j < attrs.size(); ++j)
			if (attrs[j].type == weasel::HIGHLIGHTED)
				range = attrs[j].range;

		if (range.start < range.end) {
			CSize beforeSz, hilitedSz;
			std::wstring before_str = t.substr(0, range.start);
			std::wstring hilited_str = t.substr(range.start, range.end);
			m_layout->GetTextSizeDW(before_str, before_str.length(), txtFormat, m_pDWR, &beforeSz);
			m_layout->GetTextSizeDW(hilited_str, hilited_str.length(), txtFormat, m_pDWR, &hilitedSz);

			int x = rc.left;
			int y = rc.top;

			if (range.start > 0) {
				if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
					y += beforeSz.cy + m_style.hilite_spacing;
				else
					x += beforeSz.cx + m_style.hilite_spacing;
			}
			{
				CRect rc_hi;
				if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
					rc_hi = CRect(rc.left, y, rc.right, y + hilitedSz.cy);
				else
					rc_hi = CRect(x, rc.top, x + hilitedSz.cx, rc.bottom);
				// if preedit rect size smaller than icon, fill the gap to STATUS_ICON_SIZE
				if (m_layout->ShouldDisplayStatusIcon())
				{
					if ((m_style.layout_type == UIStyle::LAYOUT_HORIZONTAL || m_style.layout_type == UIStyle::LAYOUT_VERTICAL) && hilitedSz.cy < STATUS_ICON_SIZE)
						rc_hi.InflateRect(0, (STATUS_ICON_SIZE - hilitedSz.cy) / 2);
					if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT && hilitedSz.cx < STATUS_ICON_SIZE)
						rc_hi.InflateRect((STATUS_ICON_SIZE - hilitedSz.cx) / 2, 0);
				}

				rc_hi.InflateRect(m_style.hilite_padding_x, m_style.hilite_padding_y);
				IsToRoundStruct rd = m_layout->GetTextRoundInfo();
				if (m_istorepos) {
					std::swap(rd.IsTopLeftNeedToRound, rd.IsBottomLeftNeedToRound);
					std::swap(rd.IsTopRightNeedToRound, rd.IsBottomRightNeedToRound);
				}
				_HighlightText(dc, rc_hi, m_style.hilited_back_color, m_style.hilited_shadow_color, m_style.round_corner, BackType::TEXT, rd);
			}
		}
		drawn = true;
	}
	return drawn;
}

bool WeaselPanel::_DrawCandidates(CDCHandle& dc, bool back)
{
	bool drawn = false;
	const std::vector<Text>& candidates(m_ctx.cinfo.candies);
	const std::vector<Text>& comments(m_ctx.cinfo.comments);
	const std::vector<Text>& labels(m_ctx.cinfo.labels);

	// prevent all text format nullptr
	if (m_pDWR->pTextFormat == nullptr && m_pDWR->pLabelTextFormat == nullptr && m_pDWR->pCommentTextFormat == nullptr)
	{
		_InitFontRes(true);
	}

	auto txtFormat = m_pDWR->pTextFormat;
	auto labeltxtFormat = m_pDWR->pLabelTextFormat;
	auto commenttxtFormat = m_pDWR->pCommentTextFormat;
	BackType bkType = BackType::CAND;


	CRect rect;
	// draw back color and shadow color, with gdi+
	if (back)
	{
		// if candidate_shadow_color not transparent, draw candidate shadow first
		if (COLORNOTTRANSPARENT(m_style.candidate_shadow_color))
		{
			for (auto i = 0; i < m_candidateCount && i < MAX_CANDIDATES_COUNT; ++i)
			{
				if (i == m_ctx.cinfo.highlighted) continue;	// draw non hilited candidates only 
				rect = m_layout->GetCandidateRect((int)i);
				IsToRoundStruct rd = m_layout->GetRoundInfo(i);
				if (m_istorepos) {
					rect.OffsetRect(0, m_offsetys[i]);
					ReconfigRoundInfo(rd, i, m_candidateCount);
				}
				rect.InflateRect(m_style.hilite_padding_x, m_style.hilite_padding_y);
				_HighlightText(dc, rect, 0x00000000, m_style.candidate_shadow_color, m_style.round_corner, bkType, rd);
				drawn = true;
			}
		}
		// draw non highlighted candidates, without shadow
		if (COLORNOTTRANSPARENT(m_style.candidate_back_color) || COLORNOTTRANSPARENT(m_style.candidate_border_color))	// if transparent not to draw
		{
			for (auto i = 0; i < m_candidateCount && i < MAX_CANDIDATES_COUNT; ++i) {
				if (i == m_ctx.cinfo.highlighted) continue;
				rect = m_layout->GetCandidateRect((int)i);
				IsToRoundStruct rd = m_layout->GetRoundInfo(i);
				if (m_istorepos) {
					rect.OffsetRect(0, m_offsetys[i]);
					ReconfigRoundInfo(rd, i, m_candidateCount);
				}
				rect.InflateRect(m_style.hilite_padding_x, m_style.hilite_padding_y);
				_HighlightText(dc, rect, m_style.candidate_back_color, 0x00000000, m_style.round_corner, bkType, rd, m_style.candidate_border_color);
				drawn = true;
			}
		}
		// draw highlighted back ground and shadow
		{
			rect = m_layout->GetHighlightRect();
			IsToRoundStruct rd = m_layout->GetRoundInfo(m_ctx.cinfo.highlighted);
			if (m_istorepos) {
				rect.OffsetRect(0, m_offsetys[m_ctx.cinfo.highlighted]);
				ReconfigRoundInfo(rd, m_ctx.cinfo.highlighted, m_candidateCount);
			}
			rect.InflateRect(m_style.hilite_padding_x, m_style.hilite_padding_y);
			_HighlightText(dc, rect, m_style.hilited_candidate_back_color, m_style.hilited_candidate_shadow_color, m_style.round_corner, bkType, rd, m_style.hilited_candidate_border_color);
			if (m_style.mark_text.empty() && COLORNOTTRANSPARENT(m_style.hilited_mark_color))
			{
				BYTE r = GetRValue(m_style.hilited_mark_color);
				BYTE g = GetGValue(m_style.hilited_mark_color);
				BYTE b = GetBValue(m_style.hilited_mark_color);
				BYTE alpha = (BYTE)(m_style.hilited_mark_color >> 24);
				Gdiplus::Graphics g_back(dc);
				g_back.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
				Gdiplus::Color mark_color = Gdiplus::Color::MakeARGB(alpha, r, g, b);
				Gdiplus::SolidBrush mk_brush(mark_color);
				int height = min(rect.Height() - m_style.hilite_padding_y * 2, rect.Height() - m_style.round_corner * 2);
				int width = min(rect.Width() - m_style.hilite_padding_x * 2, rect.Width() - m_style.round_corner * 2);
				if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
				{
					int x = rect.left + (rect.Width() - width) / 2;
					CRect mkrc{ x, rect.top, x + width, rect.top + m_layout->MARK_HEIGHT };
					GraphicsRoundRectPath mk_path(mkrc, 2);
					g_back.FillPath(&mk_brush, &mk_path);
				}
				else
				{
					int y = rect.top + (rect.Height() - height) / 2;
					CRect mkrc{ rect.left, y, rect.left + m_layout->MARK_WIDTH, y + height };
					GraphicsRoundRectPath mk_path(mkrc, 2);
					g_back.FillPath(&mk_brush, &mk_path);
				}
			}
			drawn = true;
		}
	}
	// draw text with direct write
	else
	{
		// begin draw candidate texts
		int label_text_color, candidate_text_color, comment_text_color;
		for (auto i = 0; i < m_candidateCount && i < MAX_CANDIDATES_COUNT; ++i)
		{
			if (i == m_ctx.cinfo.highlighted)
			{
				label_text_color = m_style.hilited_label_text_color;
				candidate_text_color = m_style.hilited_candidate_text_color;
				comment_text_color = m_style.hilited_comment_text_color;
			}
			else
			{
				label_text_color = m_style.label_text_color;
				candidate_text_color = m_style.candidate_text_color;
				comment_text_color = m_style.comment_text_color;
			}
			// Draw label
			std::wstring label = m_layout->GetLabelText(labels, (int)i, m_style.label_text_format.c_str());
			if (!label.empty()) {
				rect = m_layout->GetCandidateLabelRect((int)i);
				if (m_istorepos) rect.OffsetRect(0, m_offsetys[i]);
				_TextOut(rect, label, label.length(), label_text_color, labeltxtFormat.Get());
			}
			// Draw text
			std::wstring text = candidates.at(i).str;
			if (!text.empty()) {
				rect = m_layout->GetCandidateTextRect((int)i);
				if (m_istorepos) rect.OffsetRect(0, m_offsetys[i]);
				_TextOut(rect, text, text.length(), candidate_text_color, txtFormat.Get());
			}
			// Draw comment
			std::wstring comment = comments.at(i).str;
			if (!comment.empty() && COLORNOTTRANSPARENT(comment_text_color)) {
				rect = m_layout->GetCandidateCommentRect((int)i);
				if (m_istorepos) rect.OffsetRect(0, m_offsetys[i]);
				_TextOut(rect, comment, comment.length(), comment_text_color, commenttxtFormat.Get());
			}
			drawn = true;
		}
		// draw highlight mark
		if (!m_style.mark_text.empty() && COLORNOTTRANSPARENT(m_style.hilited_mark_color))
		{
			CRect rc = m_layout->GetHighlightRect();
			if (m_istorepos) rc.OffsetRect(0, m_offsetys[m_ctx.cinfo.highlighted]);
			rc.InflateRect(m_style.hilite_padding_x, m_style.hilite_padding_y);
			int vgap = m_layout->MARK_HEIGHT ? (rc.Height() - m_layout->MARK_HEIGHT) / 2 : 0;
			int hgap = m_layout->MARK_WIDTH ? (rc.Width() - m_layout->MARK_WIDTH) / 2 : 0;
			CRect hlRc;
			if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
				hlRc = CRect(rc.left + hgap, 
					rc.top + m_style.hilite_padding_y,
					rc.left + hgap + m_layout->MARK_WIDTH, 
					rc.top + m_style.hilite_padding_y + m_layout->MARK_HEIGHT);
			else
				hlRc = CRect(rc.left + m_style.hilite_padding_x,
					rc.top + vgap,
					rc.left + m_style.hilite_padding_x + m_layout->MARK_WIDTH, 
					rc.bottom - vgap);
			_TextOut(hlRc, m_style.mark_text, m_style.mark_text.length(), m_style.hilited_mark_color, m_pDWR->pTextFormat.Get());
		}
	}
	return drawn;
}

//draw client area
void WeaselPanel::DoPaint(CDCHandle dc/*, RECT& rect*/)
{
	// turn off WS_EX_TRANSPARENT, for better resp performance
	bool ret = ModifyStyleEx(WS_EX_TRANSPARENT, WS_EX_LAYERED);

	GetClientRect(&rcw);
	// prepare memDC
	CDCHandle hdc = ::GetDC(m_hWnd);
	CDCHandle memDC = ::CreateCompatibleDC(hdc);
	HBITMAP memBitmap = ::CreateCompatibleBitmap(hdc, rcw.Width(), rcw.Height());
	::SelectObject(memDC, memBitmap);
	ReleaseDC(hdc);
	bool drawn = false;
	if (!hide_candidates) {
		CRect auxrc = m_layout->GetAuxiliaryRect();
		CRect preeditrc = m_layout->GetPreeditRect();
		if (m_istorepos)
		{
			std::vector<CRect> rects;
			rects.reserve(m_candidateCount);
			std::vector<int> btmys;
			btmys.reserve(m_candidateCount);
			for (auto i = 0; i < m_candidateCount && i < MAX_CANDIDATES_COUNT; ++i) {
				rects.emplace_back(m_layout->GetCandidateRect(i));
				btmys.emplace_back(rects[i].bottom);
			}
			if (m_candidateCount) {
				if (!m_layout->IsInlinePreedit() && !m_ctx.preedit.str.empty())
					m_offsety_preedit = rects[m_candidateCount - 1].bottom - preeditrc.bottom;
				if (!m_ctx.aux.str.empty())
					m_offsety_aux = rects[m_candidateCount - 1].bottom - auxrc.bottom;
			}
			else {
				m_offsety_preedit = 0;
				m_offsety_aux = 0;
			}
			int base_gap = 0;
			if (!m_ctx.aux.str.empty())
				base_gap = auxrc.Height() + m_style.spacing;
			else if (!m_layout->IsInlinePreedit() && !m_ctx.preedit.str.empty())
				base_gap = preeditrc.Height() + m_style.spacing;

			for (auto i = 0; i < m_candidateCount && i < MAX_CANDIDATES_COUNT; ++i) {
				if (i == 0)
					m_offsetys[i] = btmys[m_candidateCount - i - 1] - base_gap - rects[i].bottom;
				else
					m_offsetys[i] = (rects[i - 1].top + m_offsetys[i - 1] - m_style.candidate_spacing) - rects[i].bottom;
			}
		}
		// background and candidates back, hilite back drawing start
		if ((!m_ctx.empty() && !m_style.inline_preedit) || (m_style.inline_preedit && (m_candidateCount || !m_ctx.aux.empty())))
		{
			CRect backrc = m_layout->GetContentRect();
			if (!m_style.border)
			{
				backrc.InflateRect(1, 1);
			}
			_HighlightText(memDC, backrc, m_style.back_color, m_style.shadow_color, m_style.round_corner_ex, BackType::BACKGROUND, IsToRoundStruct(), m_style.border_color);
		}
		if (!m_ctx.aux.str.empty())
		{
			if (m_istorepos)
				auxrc.OffsetRect(0, m_offsety_aux);
			drawn |= _DrawPreeditBack(m_ctx.aux, memDC, auxrc);
		}
		if (!m_layout->IsInlinePreedit() && !m_ctx.preedit.str.empty())
		{
			if (m_istorepos)
				preeditrc.OffsetRect(0, m_offsety_preedit);
			drawn |= _DrawPreeditBack(m_ctx.preedit, memDC, preeditrc);
		}
		if (m_candidateCount)
			drawn |= _DrawCandidates(memDC, true);
		// background and candidates back, hilite back drawing end

		// begin  texts drawing, if pRenderTarget failed, force to reinit directwrite resources
		if (FAILED(m_pDWR->pRenderTarget->BindDC(memDC, &rcw)))
		{
			_InitFontRes(true);
			m_pDWR->pRenderTarget->BindDC(memDC, &rcw);
		}
		m_pDWR->pRenderTarget->BeginDraw();
		// draw auxiliary string
		if (!m_ctx.aux.str.empty())
			drawn |= _DrawPreedit(m_ctx.aux, memDC, auxrc);
		// draw preedit string
		if (!m_layout->IsInlinePreedit() && !m_ctx.preedit.str.empty())
			drawn |= _DrawPreedit(m_ctx.preedit, memDC, preeditrc);
		// draw candidates string
		if (m_candidateCount)
			drawn |= _DrawCandidates(memDC);
		m_pDWR->pRenderTarget->EndDraw();
		// end texts drawing

		// status icon (I guess Metro IME stole my idea :)
		if (m_layout->ShouldDisplayStatusIcon()) {
			// decide if custom schema zhung icon to show
			LoadIconNecessary(m_current_zhung_icon, m_style.current_zhung_icon, m_iconEnabled, IDI_ZH);
			LoadIconNecessary(m_current_ascii_icon, m_style.current_ascii_icon, m_iconAlpha, IDI_EN);
			LoadIconNecessary(m_current_half_icon, m_style.current_half_icon, m_iconHalf, IDI_HALF_SHAPE);
			LoadIconNecessary(m_current_full_icon, m_style.current_full_icon, m_iconFull, IDI_FULL_SHAPE);
			CRect iconRect(m_layout->GetStatusIconRect());
			if (m_istorepos && !m_ctx.aux.str.empty())
				iconRect.OffsetRect(0, m_offsety_aux);
			else if (m_istorepos && !m_layout->IsInlinePreedit() && !m_ctx.preedit.str.empty())
				iconRect.OffsetRect(0, m_offsety_preedit);

			CIcon& icon(m_status.disabled ? m_iconDisabled : m_status.ascii_mode ? m_iconAlpha : 
				(m_status.type == SCHEMA ? m_iconEnabled : (m_status.full_shape ? m_iconFull : m_iconHalf)));
			memDC.DrawIconEx(iconRect.left, iconRect.top, icon, 0, 0);
			drawn = true;
		}
	}
	_LayerUpdate(rcw, memDC);

	// clean objs
	::DeleteDC(memDC);
	::DeleteObject(memBitmap);
}

void WeaselPanel::_LayerUpdate(const CRect& rc, CDCHandle dc)
{
	HDC ScreenDC = ::GetDC(NULL);
	CRect rect;
	GetWindowRect(&rect);
	POINT WindowPosAtScreen = { rect.left, rect.top };
	POINT PointOriginal = { 0, 0 };
	SIZE sz = { rc.Width(), rc.Height() };

	BLENDFUNCTION bf = { AC_SRC_OVER, 0, 0XFF, AC_SRC_ALPHA };
	auto result = UpdateLayeredWindow(m_hWnd, ScreenDC, &WindowPosAtScreen, &sz, dc, &PointOriginal, RGB(0, 0, 0), &bf, ULW_ALPHA);
	ReleaseDC(ScreenDC);
}

LRESULT WeaselPanel::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	GetWindowRect(&m_inputPos);
	Refresh();
	return TRUE;
}

LRESULT WeaselPanel::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_sticky = false;
	delete m_layout;
	m_layout = nullptr;
	m_reversed = false;
	return 0;
}

LRESULT WeaselPanel::OnDpiChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Refresh();
	return 0;
}

void WeaselPanel::MoveTo(RECT const& rc)
{
	if (!m_layout)	return;			// avoid handling nullptr in _RepositionWindow

	// if ascii_tip_follow_cursor set, move tip icon to mouse cursor
	if (m_style.ascii_tip_follow_cursor && m_ctx.aux.empty() && (!m_status.composing) && m_layout->ShouldDisplayStatusIcon())	// ascii icon
	{
		POINT p;
		::GetCursorPos(&p);
		RECT irc{ p.x - STATUS_ICON_SIZE, p.y - STATUS_ICON_SIZE, p.x, p.y };
		m_inputPos = irc;
		m_oinputPos = irc;
		_RepositionWindow(true);
		RedrawWindow();
	}
	else if (rc.left != m_inputPos.left || rc.top != m_inputPos.top || !m_ctx.aux.empty() || (m_ctx.empty() && m_layout->ShouldDisplayStatusIcon()))
	{
		// in some apps like word 2021, with inline_preedit set,
		// bottom of rc would flicker 1 px or 2, make the candidate flickering
		m_inputPos = rc;
		m_oinputPos = m_inputPos;
		// buffer current m_istorepos status
		bool m_istorepos_buf = m_istorepos;
		// with parameter to avoid vertical flicker
		_RepositionWindow(true);
		// m_istorepos status changed by _RepositionWindow, or tips to show,
		// redrawing is required
		if (m_istorepos != m_istorepos_buf || !m_ctx.aux.empty() || (m_ctx.empty() && m_layout->ShouldDisplayStatusIcon()))
		{
			RedrawWindow();
		}
	}
}

void WeaselPanel::_RepositionWindow(bool adj)
{
	RECT rcWorkArea;
	memset(&rcWorkArea, 0, sizeof(rcWorkArea));
	HMONITOR hMonitor = MonitorFromRect(m_inputPos, MONITOR_DEFAULTTONEAREST);
	if (hMonitor) {
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(hMonitor, &info)) {
			rcWorkArea = info.rcWork;
		}
	}
	RECT rcWindow;
	GetWindowRect(&rcWindow);
	int width = (rcWindow.right - rcWindow.left);
	int height = (rcWindow.bottom - rcWindow.top);
	// keep panel visible
	rcWorkArea.right -= width;
	rcWorkArea.bottom -= height;
	static int saved_bottom{ rcWorkArea.bottom };
	int x = m_inputPos.left;
	int y = m_inputPos.bottom;
	// m_reversed = false;
	if (m_style.shadow_radius > 0)
	{
		x -= (m_style.shadow_offset_x >= 0 || COLORTRANSPARENT(m_style.shadow_color)) ? m_layout->offsetX : (m_layout->offsetX / 2);

		if (adj)
		{
			y -= (m_style.shadow_offset_y >= 0 || COLORTRANSPARENT(m_style.shadow_color)) ? m_layout->offsetY : (m_layout->offsetY / 2);
		}
		// for vertical text layout, flow right to left, make window left side
		if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT && !m_style.vertical_text_left_to_right)
		{
			x += m_layout->offsetX - width;
			if (m_style.shadow_offset_x < 0)
			{
				x += m_layout->offsetX;
			}
		}
	}
	if (adj) m_istorepos = false;
	if (x > rcWorkArea.right) x = rcWorkArea.right;		// over workarea right
	if (x < rcWorkArea.left) x = rcWorkArea.left;		// over workarea left
	// show panel above the input focus if we're around the bottom
	if (m_reversed)
	{
		y -= saved_bottom - rcWorkArea.bottom;
	}
	saved_bottom = rcWorkArea.bottom;

	if (rcWorkArea.bottom < y || m_sticky)
	{
		if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL || m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT)
			m_sticky = true;
		y = m_inputPos.top - height;					// over workarea bottom
		m_reversed = true;

		if (m_style.shadow_radius && m_style.shadow_offset_y > 0)
			y -= m_style.shadow_offset_y;

		m_istorepos = (m_style.vertical_auto_reverse && m_style.layout_type == UIStyle::LAYOUT_VERTICAL);
		if (m_style.shadow_radius > 0)
			y += (m_style.shadow_offset_y < 0 || COLORTRANSPARENT(m_style.shadow_color)) ? m_layout->offsetY : (m_layout->offsetY / 2);
	}

	if (y < rcWorkArea.top) y = rcWorkArea.top;		// over workarea top
	// memorize adjusted position (to avoid window bouncing on height change)
	m_inputPos.bottom = y;
	SetWindowPos(HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
}

void WeaselPanel::_TextOut(CRect const& rc, std::wstring_view psz, size_t cch, int inColor, IDWriteTextFormat1* const pTextFormat)
{
	if (pTextFormat == NULL) return;
	float r = (float)(GetRValue(inColor)) / 255.0f;
	float g = (float)(GetGValue(inColor)) / 255.0f;
	float b = (float)(GetBValue(inColor)) / 255.0f;
	float alpha = (float)((inColor >> 24) & 255) / 255.0f;
	HRESULT hr{ S_OK };

	if (m_pDWR->m_pBrush == nullptr)
	{
		hr = m_pDWR->CreateBrush(D2D1::ColorF(r, g, b, alpha));
		if (FAILED(hr))
			MessageBox(L"Failed CreateBrush", L"Info");
	}
	else
	{
		m_pDWR->SetBrushColor(D2D1::ColorF(r, g, b, alpha));
	}

	if (NULL != m_pDWR->m_pBrush && NULL != pTextFormat) {
		m_pDWR->CreateTextLayout(psz.data(), (int)cch, pTextFormat, (float)rc.Width(), (float)rc.Height());
		if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT) {
			DWRITE_FLOW_DIRECTION flow = m_style.vertical_text_left_to_right ? DWRITE_FLOW_DIRECTION_LEFT_TO_RIGHT : DWRITE_FLOW_DIRECTION_RIGHT_TO_LEFT;
			m_pDWR->SetLayoutReadingDirection(DWRITE_READING_DIRECTION_TOP_TO_BOTTOM);
			m_pDWR->SetLayoutFlowDirection(flow);
		}

		// offsetx for font glyph over left
		float offsetx = rc.left;
		float offsety = rc.top;
		// prepare for space when first character overhanged
		DWRITE_OVERHANG_METRICS omt;
		m_pDWR->GetLayoutOverhangMetrics(&omt);
		if (m_style.layout_type != UIStyle::LAYOUT_VERTICAL_TEXT && omt.left > 0)
			offsetx += omt.left;
		if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT && omt.top > 0)
			offsety += omt.top;

		if (m_pDWR->pTextLayout != NULL) {
			m_pDWR->DrawTextLayoutAt({ offsetx, offsety });
#if 0
			D2D1_RECT_F rectf = D2D1::RectF(offsetx, offsety, offsetx + rc.Width(), offsety + rc.Height());
			pDWR->DrawRect(&rectf);
#endif
		}
		m_pDWR->ResetLayout();
	}
}


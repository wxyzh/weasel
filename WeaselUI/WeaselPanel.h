#pragma once
#include <WeaselCommon.h>
#include <WeaselUI.h>
#include "StandardLayout.h"
#include "Layout.h"
#include "GdiplusBlur.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#define USE_MOUSE_EVENTS
#define TRANS_COLOR		0x00000000

using namespace weasel;

typedef CWinTraits<WS_POPUP/* | WS_CLIPSIBLINGS | WS_DISABLED*/, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT> CWeaselPanelTraits;

enum class BackType
{
	TEXT = 0,
	CAND = 1,
	BACKGROUND = 2	// background
};

class WeaselPanel :
	public CWindowImpl<WeaselPanel, CWindow, CWeaselPanelTraits>,
	public CDoubleBufferImpl<WeaselPanel>
{
public:
	BEGIN_MSG_MAP(WeaselPanel)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_DPICHANGED, OnDpiChanged)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLeftClicked)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLeftReleased)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_MOUSEHOVER, OnMouseHover)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		CHAIN_MSG_MAP(CDoubleBufferImpl<WeaselPanel>)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDpiChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLeftClicked(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLeftReleased(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);	

	WeaselPanel(weasel::UI& ui);
	~WeaselPanel();

	void MoveTo(RECT const& rc);
	void Refresh();
	void DoPaint(CDCHandle dc);
	bool GetIsReposition() { return m_istorepos; }
	void SetCaretFollowing(const bool following) { m_following = following; }

private:
	void _InitFontRes(bool forced = false);
	void _CaptureRect(CRect& rect);
	bool m_mouse_entry = false;
	void _CreateLayout();
	void _ResizeWindow();
	void _RepositionWindow(bool adj = false);
	bool _DrawPreedit(weasel::Text const& text, CDCHandle dc, CRect const& rc);
	bool _DrawPreeditBack(weasel::Text const& text, CDCHandle dc, CRect const& rc);
	bool _DrawCandidates(CDCHandle& dc, bool back = false);
	void _HighlightText(CDCHandle& dc, const CRect& rc, const COLORREF& color, const COLORREF& shadowColor, int radius,
		const BackType& type = BackType::TEXT, const IsToRoundStruct& rd = IsToRoundStruct(), const COLORREF& bordercolor = TRANS_COLOR);
	void _TextOut(CRect const& rc, std::wstring_view psz, size_t cch, int inColor, IDWriteTextFormat1* const pTextFormat = nullptr);

	void _LayerUpdate(const CRect& rc, CDCHandle dc);

private:
	std::shared_ptr<weasel::Layout> m_layout;
	weasel::Context& m_ctx;
	weasel::Context& m_octx;
	weasel::Status& m_status;
	weasel::UIStyle& m_style;
	weasel::UIStyle& m_ostyle;

	CRect m_inputPos;
	CRect m_oinputPos;
	int  m_offsetys[MAX_CANDIDATES_COUNT];	// offset y for candidates when vertical layout over bottom
	int  m_offsety_preedit;
	int  m_offsety_aux;
	bool m_istorepos;

	CIcon m_iconDisabled;
	CIcon m_iconEnabled;
	CIcon m_iconAlpha;
	CIcon m_iconFull;
	CIcon m_iconHalf;
	std::wstring m_current_zhung_icon;
	std::wstring m_current_ascii_icon;
	std::wstring m_current_half_icon;
	std::wstring m_current_full_icon;
	// for gdiplus drawings
	Gdiplus::GdiplusStartupInput _m_gdiplusStartupInput;
	ULONG_PTR _m_gdiplusToken;

	UINT dpi;

	CRect rcw;
	BYTE m_candidateCount;

	bool hide_candidates;
	// for multi font_face & font_point
	PDWR m_pDWR;
	std::function<void(int* const, int* const, bool* const, bool* const)>& _UICallback;
	std::function<void(const RECT&)>& _SetRectCallback;

	bool m_following{ true };
	bool m_holder{};
	bool m_sticky{};
	bool m_reversed{};
};
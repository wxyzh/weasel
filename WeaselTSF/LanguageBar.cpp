module;
#include "stdafx.h"
#include "Globals.h"
#include <WeaselUI.h>
#include "resource.h"
// #include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module LanguageBar;
import WeaselTSF;
import CandidateList;

static const DWORD LANGBARITEMSINK_COOKIE = 0x42424242;

static void HMENU2ITfMenu(HMENU hMenu, ITfMenu *pTfMenu)
{
	/* NOTE: Only limited functions are supported */
	int N = GetMenuItemCount(hMenu);
#ifdef TEST
	LOG(INFO) << std::format("From HMENU2ITfMenu.");
#endif // TEST
	for (int i = 0; i < N; i++)
	{
		MENUITEMINFO mii;
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_FTYPE | MIIM_ID;
		mii.dwTypeData = NULL;
		if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
		{
			UINT id = mii.wID;
			if (mii.fType == MFT_SEPARATOR)
				pTfMenu->AddMenuItem(id, TF_LBMENUF_SEPARATOR, NULL, NULL, NULL, 0, NULL);
			else if (mii.fType == MFT_STRING)
			{
				mii.dwTypeData = (LPWSTR) malloc(sizeof(WCHAR) * (mii.cch + 1));
				mii.cch++;
				if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
				{
					pTfMenu->AddMenuItem(id, 0, NULL, NULL, mii.dwTypeData, mii.cch, NULL);
				}
				free(mii.dwTypeData);
			}
		}
	}
}

CLangBarItemButton::CLangBarItemButton(WeaselTSF& textService, REFGUID guid, weasel::UIStyle& style)
	: _textService{ textService }, _status(0), _style(style), _current_schema_zhung_icon(), _current_schema_ascii_icon()
{
	DllAddRef();

	_pLangBarItemSink = NULL;
	_cRef = 1;
	_guid = guid;
	ascii_mode = false;
}

CLangBarItemButton::~CLangBarItemButton()
{
	DllRelease();
}

STDAPI CLangBarItemButton::QueryInterface(REFIID riid, void **ppvObject)
{
	if (ppvObject == NULL)
		return E_INVALIDARG;

	*ppvObject = NULL;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfLangBarItem) || IsEqualIID(riid, IID_ITfLangBarItemButton))
		*ppvObject = (ITfLangBarItemButton *) this;
	else if (IsEqualIID(riid, IID_ITfSource))
		*ppvObject = (ITfSource *) this;

	if (*ppvObject)
	{
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDAPI_(ULONG) CLangBarItemButton::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) CLangBarItemButton::Release()
{
	LONG cr = --_cRef;
	assert(_cRef >= 0);
	if (_cRef == 0)
		delete this;
	return cr;
}

STDAPI CLangBarItemButton::GetInfo(TF_LANGBARITEMINFO *pInfo)
{
	pInfo->clsidService = c_clsidTextService;
	pInfo->guidItem = _guid;
	pInfo->dwStyle = TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_SHOWNINTRAYONLY;
	pInfo->ulSort = 1;
	lstrcpyW(pInfo->szDescription, L"WeaselTSF Button");
	return S_OK;
}

STDAPI CLangBarItemButton::GetStatus(DWORD *pdwStatus)
{
	*pdwStatus = _status;
	return S_OK;
}

STDAPI CLangBarItemButton::Show(BOOL fShow)
{
	SetLangbarStatus(TF_LBI_STATUS_HIDDEN, fShow ? FALSE : TRUE);
	return S_OK;
}

STDAPI CLangBarItemButton::GetTooltipString(BSTR *pbstrToolTip)
{
	*pbstrToolTip = SysAllocString(L"左键切换模式，右键打开菜单");
	return (*pbstrToolTip == NULL)? E_OUTOFMEMORY: S_OK;
}

STDAPI CLangBarItemButton::OnClick(TfLBIClick click, POINT pt, const RECT *prcArea)
{
	if (click == TF_LBI_CLK_LEFT)
	{
		_textService._HandleLangBarMenuSelect(ascii_mode ? ID_WEASELTRAY_DISABLE_ASCII : ID_WEASELTRAY_ENABLE_ASCII);
		ascii_mode = !ascii_mode;
		if (_pLangBarItemSink) {
			_pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
		}
	}
	else if (click == TF_LBI_CLK_RIGHT)
	{
		RightClick(pt);
	}
	return S_OK;
}

STDAPI CLangBarItemButton::InitMenu(ITfMenu *pMenu)
{
#ifdef TEST
	LOG(INFO) << std::format("From CLangBarItemButton::InitMenu. _daemon_enable = {}", _textService.GetBit(WeaselFlag::DAEMON_ENABLE));
#endif // TEST
	HMENU menu = LoadMenuW(g_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP));
	HMENU popupMenu = GetSubMenu(menu, 0);
	HMENU2ITfMenu(popupMenu, pMenu);
	DestroyMenu(menu);
	return S_OK;
}

STDAPI CLangBarItemButton::OnMenuSelect(UINT wID)
{
	_textService._HandleLangBarMenuSelect(wID);
	return S_OK;
}

STDAPI CLangBarItemButton::GetIcon(HICON *phIcon)
{
	if (ascii_mode)
	{
		if(_style.current_ascii_icon.empty())
			*phIcon = (HICON)LoadImageW(
					g_hInst,
					MAKEINTRESOURCEW(IDI_EN),
					IMAGE_ICON,
					GetSystemMetrics(SM_CXSMICON),
					GetSystemMetrics(SM_CYSMICON),
					LR_SHARED);
		else
			*phIcon = (HICON)LoadImageW(
					NULL,
					_style.current_ascii_icon.c_str(),
					IMAGE_ICON,
					GetSystemMetrics(SM_CXSMICON),
					GetSystemMetrics(SM_CYSMICON),
					LR_LOADFROMFILE);
	}
	else
	{
		if( _style.current_zhung_icon.empty()) 
			*phIcon = (HICON)LoadImageW(
					g_hInst,
					MAKEINTRESOURCEW(IDI_ZH),
					IMAGE_ICON,
					GetSystemMetrics(SM_CXSMICON),
					GetSystemMetrics(SM_CYSMICON),
					LR_SHARED);
		else
			*phIcon = (HICON)LoadImageW(
					NULL,
					_style.current_zhung_icon.c_str(),
					IMAGE_ICON,
					GetSystemMetrics(SM_CXSMICON),
					GetSystemMetrics(SM_CYSMICON),
					LR_LOADFROMFILE);
	}
	return (*phIcon == NULL)? E_FAIL: S_OK;
}

STDAPI CLangBarItemButton::GetText(BSTR *pbstrText)
{
	*pbstrText = SysAllocString(L"WeaselTSF Button");
	return (*pbstrText == NULL) ? E_OUTOFMEMORY : S_OK;
}

STDAPI CLangBarItemButton::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
	if (!IsEqualIID(riid, IID_ITfLangBarItemSink))
		return CONNECT_E_CANNOTCONNECT;
	if (_pLangBarItemSink != NULL)
		return CONNECT_E_ADVISELIMIT;

	if (punk->QueryInterface(IID_ITfLangBarItemSink, (LPVOID *) &_pLangBarItemSink) != S_OK)
	{
		_pLangBarItemSink = NULL;
		return E_NOINTERFACE;
	}
	*pdwCookie = LANGBARITEMSINK_COOKIE;
	return S_OK;
}

STDAPI CLangBarItemButton::UnadviseSink(DWORD dwCookie)
{
	if (dwCookie != LANGBARITEMSINK_COOKIE || _pLangBarItemSink == NULL)
		return CONNECT_E_NOCONNECTION;
	_pLangBarItemSink == NULL;
	return S_OK;
}

void CLangBarItemButton::UpdateWeaselStatus(weasel::Status stat)
{
	if (stat.ascii_mode != ascii_mode) {
		ascii_mode = stat.ascii_mode;
		if (_pLangBarItemSink) {
			_pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
		}
	}
	if (_current_schema_zhung_icon != _style.current_zhung_icon) {
		_current_schema_zhung_icon = _style.current_zhung_icon;
		if (_pLangBarItemSink) {
			_pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
		}
	}
	if (_current_schema_ascii_icon != _style.current_ascii_icon) {
		_current_schema_ascii_icon = _style.current_ascii_icon;
		if (_pLangBarItemSink) {
			_pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
		}
	}
}

void CLangBarItemButton::SetLangbarStatus(DWORD dwStatus, BOOL fSet)
{
	BOOL isChange = FALSE;

	if (fSet)
	{
		if (!(_status & dwStatus))
		{
			_status |= dwStatus;
			isChange = TRUE;
		}
	}
	else
	{
		if (_status & dwStatus)
		{
			_status &= ~dwStatus;
			isChange = TRUE;
		}
	}

	if (isChange && _pLangBarItemSink)
	{
		_pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
	}

	return;
}

void CLangBarItemButton::RightClick(POINT& pt)
{
	/* Open menu */
	HWND hwnd = _textService._GetFocusedContextWindow();
	if (hwnd != NULL)
	{
		// 全局菜单
		HMENU menu = LoadMenuW(g_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP));
		// 0级弹出菜单
		HMENU mainMenu = GetSubMenu(menu, 0);

		// 1级弹出菜单
		HMENU firstPopupMenu = GetSubMenu(mainMenu, 0);
		CheckMenuItem(firstPopupMenu, ID_WEASELTRAY_DAEMON_ENABLE,
			MF_BYCOMMAND | (_textService.GetBit(WeaselFlag::DAEMON_ENABLE) ? MF_CHECKED : MF_UNCHECKED));

		std::wstring temp(16, 0);
		MENUITEMINFO mii;
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING | MIIM_SUBMENU | MIIM_CHECKMARKS;
		mii.dwTypeData = temp.data();
		mii.cch = temp.capacity();
		// 2级弹出菜单
		if (GetMenuItemInfo(firstPopupMenu, 0, TRUE, &mii))
		{
			temp = temp.data();
			temp[4] = _textService.GetBit(WeaselFlag::FULL_SHAPE) ? L'全' : L'半';
			ModifyMenu(mii.hSubMenu, _textService.GetBit(WeaselFlag::FULL_SHAPE) ? 1 : 0, 
				MF_BYPOSITION | MF_CHECKED, 0, _textService.GetBit(WeaselFlag::FULL_SHAPE) ? L"全角" : L"半角");
		}
		SetMenuItemInfo(firstPopupMenu, 0, true, &mii);

		CheckMenuItem(firstPopupMenu, ID_STYLE_CARET_FOLLOWING,
			MF_BYCOMMAND | (_textService.GetBit(WeaselFlag::CARET_FOLLOWING) ? MF_CHECKED : MF_UNCHECKED));
		
		_textService.UpdateGlobalCompartment(false);
		CheckMenuItem(firstPopupMenu, ID_STYLE_PRESERVED_KEY_SWITCH,
			MF_BYCOMMAND | (_textService.GetBit(WeaselFlag::PRESERVED_KEY_SWITCH) ? MF_CHECKED : MF_UNCHECKED));

		UINT wID = TrackPopupMenuEx(mainMenu, TPM_CENTERALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_HORPOSANIMATION, pt.x, pt.y - 32, hwnd, NULL);
#ifdef TEST
		LOG(INFO) << std::format("From CLangBarItemButton::OnClick. _daemon_enable = {}, wID = {}, var.lval = {:#x}", _textService.GetBit(WeaselFlag::DAEMON_ENABLE), wID, var.lVal);
#endif // TEST			
		DestroyMenu(menu);
		_textService._HandleLangBarMenuSelect(wID);
	}
}

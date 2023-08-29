module;
#include "stdafx.h"
#include "Globals.h"
#include "ctffunc.h"
#include <format>
#include <shellapi.h>
#include "resource.h"
// #include "test.h"
#ifdef TEST
#ifdef _M_X64
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif
#endif // TEST
module WeaselTSF;
import LanguageBar;
import CandidateList;
import Compartment;
import WeaselUtility;

bool WeaselTSF::execute(std::wstring_view cmd, std::wstring_view args)
{
	return (int)ShellExecuteW(nullptr, NULL, cmd.data(), args.data(), NULL, SW_SHOWNORMAL) > 32;
}

bool WeaselTSF::explore(std::wstring_view path)
{
	return (int)ShellExecuteW(nullptr, L"open", L"explorer", path.data(), NULL, SW_SHOWNORMAL) > 32;
}

void WeaselTSF::_HandleLangBarMenuSelect(UINT wID)
{
	bool ret{};
	switch (wID)
	{
	case ID_WEASELTRAY_SETTINGS:
		ret = execute(std::format(LR"({}\WeaselDeployer.exe)", WeaselRootPath()), nullptr);
		if (!ret)
			goto DEFAULT;
		break;

	case ID_WEASELTRAY_DICT_MANAGEMENT:
		ret = execute(std::format(LR"({}\WeaselDeployer.exe)", WeaselRootPath()), LR"(/dict)");
		if (!ret)
			goto DEFAULT;
		break;

	case ID_WEASELTRAY_USERCONFIG:
		ret = explore(WeaselUserDataPath());
		if (!ret)
			goto DEFAULT;
		break;

	case ID_WEASELTRAY_INSTALLDIR:
		ret = explore(WeaselRootPath());
		if (!ret)
			goto DEFAULT;
		break;

	case ID_WEASELTRAY_DAEMON_ENABLE:
	{
		HRESULT hr;
		if (_pGlobalCompartmentDaemon)
		{
			VARIANT var{};
			var.vt = VT_I4;
			if (GetBit(0))						// _bitset[0]: _daemon_enable
			{
				var.lVal = 0xFC00;
			}
			else
			{
				var.lVal = 0xFC01;
			}
			hr = _pGlobalCompartmentDaemon->SetValue(_tfClientId, &var);
		}
#ifdef TEST
#ifdef _M_X64
		LOG(INFO) << std::format("From WeaselTSF::_HandleLangBarMenuSelect. hr = {:#x}", (size_t)hr);
#endif // _M_X64
#endif // TEST
	}
	break;

	case ID_WEASELTRAY_HALF_SHAPE:
		ReSetBit(6);
		goto DEFAULT;

	case ID_WEASELTRAY_FULL_SHAPE:
		SetBit(6);

	DEFAULT:
	default:
		m_client.TrayCommand(wID);
	}
}

HWND WeaselTSF::_GetFocusedContextWindow()
{
	HWND hwnd = NULL;
	ITfDocumentMgr* pDocMgr;
	if (_pThreadMgr->GetFocus(&pDocMgr) == S_OK && pDocMgr != NULL)
	{
		ITfContext* pContext;
		if (pDocMgr->GetTop(&pContext) == S_OK && pContext != NULL)
		{
			ITfContextView* pContextView;
			if (pContext->GetActiveView(&pContextView) == S_OK && pContextView != NULL)
			{
				pContextView->GetWnd(&hwnd);
				pContextView->Release();
			}
			pContext->Release();
		}
		pDocMgr->Release();
	}

	if (hwnd == NULL)
	{
		HWND hwndForeground = GetForegroundWindow();
		if (GetWindowThreadProcessId(hwndForeground, NULL) == GetCurrentThreadId())
			hwnd = hwndForeground;
	}

	return hwnd;
}

BOOL WeaselTSF::_InitLanguageBar()
{
	com_ptr<ITfLangBarItemMgr> pLangBarItemMgr;
	BOOL fRet = FALSE;

	if (_pThreadMgr->QueryInterface(&pLangBarItemMgr) != S_OK)
		return FALSE;

	if ((_pLangBarButton = new CLangBarItemButton(*this, GUID_LBI_INPUTMODE, _cand->style())) == NULL)
		return FALSE;

	if (pLangBarItemMgr->AddItem(_pLangBarButton) != S_OK)
	{
		_pLangBarButton = NULL;
		return FALSE;
	}

	_pLangBarButton->Show(TRUE);
	fRet = TRUE;

	return fRet;
}

void WeaselTSF::_UninitLanguageBar()
{
	com_ptr<ITfLangBarItemMgr> pLangBarItemMgr;

	if (_pLangBarButton == NULL)
		return;

	if (_pThreadMgr->QueryInterface(&pLangBarItemMgr) == S_OK)
	{
		pLangBarItemMgr->RemoveItem(_pLangBarButton);
	}

	_pLangBarButton = NULL;
}

void WeaselTSF::_UpdateLanguageBar(weasel::Status& stat)
{
	if (!_pLangBarButton) return;

	DWORD flags{};
	_pCompartmentConversion->_GetCompartmentDWORD(flags);
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::_UpdateLanguageBar. flags = {:#x}, ascii_mode = {}, full_shape = {}, ascii_punct = {}, _InitInputMethodState = {}", flags, stat.ascii_mode, stat.full_shape, stat.ascii_punct, !GetBit(4));
#endif // _M_X64
#endif // TEST

	if (!GetBit(4))								// _bitset[4]: _InitInputMethodState
	{
		SetBit(4);								// _bitset[4]: _InitInputMethodState
		SetBit(1, _status.ascii_mode);			// _bitset[1]: _ascii_mode
		SetBit(2, _status.full_shape);			// _bitset[2]: _full_mode
		SetBit(3, _status.ascii_punct);			// _bitset[3]: _ascii_punct

		if (stat.ascii_mode)
		{
			flags &= (~TF_CONVERSIONMODE_NATIVE);
		}
		else
		{
			flags |= TF_CONVERSIONMODE_NATIVE;
		}

		if (stat.full_shape)
		{
			flags |= TF_CONVERSIONMODE_FULLSHAPE;
		}
		else
		{
			flags &= (~TF_CONVERSIONMODE_FULLSHAPE);
		}

		if (stat.ascii_punct)
		{
			flags |= TF_CONVERSIONMODE_SYMBOL;
		}
		else
		{
			flags &= (~TF_CONVERSIONMODE_SYMBOL);
		}
	}
	else
	{
		if (GetBit(1) != stat.ascii_mode)		// _bitset[1]: _ascii_mode
		{
			SetBit(1, stat.ascii_mode);			// _bitset[1]: _ascii_mode
			if (stat.ascii_mode)
			{
				flags &= (~TF_CONVERSIONMODE_NATIVE);
			}
			else
			{
				flags |= TF_CONVERSIONMODE_NATIVE;
			}
		}
		else if (GetBit(2) != stat.full_shape)	// _bitset[2]: _full_mode
		{
			SetBit(2, stat.ascii_mode);			// _bitset[2]: _full_mode
			if (stat.full_shape)
			{
				flags |= TF_CONVERSIONMODE_FULLSHAPE;
			}
			else
			{
				flags &= (~TF_CONVERSIONMODE_FULLSHAPE);
			}
		}
		else if (GetBit(3) != stat.ascii_punct)	// _bitset[3]: _ascii_punct
		{
			SetBit(3, stat.ascii_punct);		// _bitset[3]: _ascii_punct
			if (stat.ascii_punct)
			{
				flags |= TF_CONVERSIONMODE_SYMBOL;
			}
			else
			{
				flags &= (~TF_CONVERSIONMODE_SYMBOL);
			}
		}
	}

#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::_UpdateLanguageBar. flags = {:#x}", flags);
#endif // _M_X64
#endif // TEST

	// _SetCompartmentDWORD(flags, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
	_pCompartmentConversion->_SetCompartmentDWORD(flags);

	_pLangBarButton->UpdateWeaselStatus(stat);
}

void WeaselTSF::_ShowLanguageBar(BOOL show)
{
	if (!_pLangBarButton) return;
	_pLangBarButton->Show(show);

}

void WeaselTSF::_EnableLanguageBar(BOOL enable)
{
	if (!_pLangBarButton) return;
	_pLangBarButton->SetLangbarStatus(TF_LBI_STATUS_DISABLED, !enable);
}
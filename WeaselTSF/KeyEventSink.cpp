module;
#include "stdafx.h"
#include "Globals.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;
import WeaselIPC;
import KeyEvent;
import CandidateList;

void WeaselTSF::_ProcessKeyEvent(WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
	if (!_IsKeyboardOpen())
		return;

	_EnsureServerConnected();
	weasel::KeyEvent ke;
	GetKeyboardState(_lpbKeyState);
	if (!ConvertKeyEvent(wParam, lParam, _lpbKeyState, ke))
	{
		/* Unknown key event */
		*pfEaten = FALSE;
	}
	else
	{
		*pfEaten = (BOOL)m_client.ProcessKeyEvent(ke);
		switch ((unsigned)ke)
		{
		case 0xFFE5:			// Caps Lock down
			if (_IsComposing())
			{
				SetBit(WeaselFlag::COMPOSITION_WITH_CAPSLOCK);
			}
			break;

		case 0x4002'FFE5:		// Caps Lock up
			if (GetBit(WeaselFlag::COMPOSITION_WITH_CAPSLOCK))
			{
				ResetBit(WeaselFlag::COMPOSITION_WITH_CAPSLOCK);
				unsigned send{};
				std::array<INPUT, 2> inputs;

				inputs[0].type = INPUT_KEYBOARD;
				inputs[0].ki = { VK_CAPITAL, 0x14, 0, 0, 0 };

				inputs[1].type = INPUT_KEYBOARD;
				inputs[1].ki = { VK_CAPITAL, 0x14, KEYEVENTF_KEYUP, 0, 0 };
				send = SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
			}
			break;
		}
		if (!(*pfEaten) && !_fTestKeyDownPending && !(lParam >> 31) && (isdigit(ke.keycode) || ispunct(ke.keycode)))
		{
			SetBit(WeaselFlag::EATEN);
			_inputKey = ke.keycode;
			*pfEaten = true;
		}
#ifdef TEST
		LOG(INFO) << std::format("From WeaselTSF::_ProcessKeyEvent. ke = 0x{:X}, Caps_Lock = {}", (unsigned)ke, GetBit(WeaselFlag::COMPOSITION_WITH_CAPSLOCK));
#endif // TEST
	}
}

STDAPI WeaselTSF::OnSetFocus(BOOL fForeground)
{
	if (fForeground)
	{
		m_client.FocusIn();
#ifdef TEST
		LOG(INFO) << std::format("From OnSetFocus. fForeground = {}", fForeground);
#endif // TEST
	}
	else {
		m_client.FocusOut();
		_AbortComposition();
	}

	return S_OK;
}

/* Some apps sends strange OnTestKeyDown/OnKeyDown combinations:
 *  Some sends OnKeyDown() only. (QQ2012)
 *  Some sends multiple OnTestKeyDown() for a single key event. (MS WORD 2010 x64)
 *
 * We assume every key event will eventually cause a OnKeyDown() call.
 * We use _fTestKeyDownPending to omit multiple OnTestKeyDown() calls,
 *  and for OnKeyDown() to check if the key has already been sent to the server.
 */

STDAPI WeaselTSF::OnTestKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
#ifdef TEST
	LOG(INFO) << std::format("From OnTestKeyDown. wParam = {:#x}, lParam = {:#x}, pContext = {:#x}", wParam, lParam, (size_t)pContext);
#endif // TEST
	_fTestKeyUpPending = false;
	ResetBit(WeaselFlag::FIRST_KEY_COMPOSITION);
	if (_fTestKeyDownPending)
	{
		*pfEaten = TRUE;
		return S_OK;
	}
	if (GetBit(WeaselFlag::GAME_MODE))
	{
		if (wParam == 0x0C)
		{
			*pfEaten = true;
			_fTestKeyDownPending = true;
			SetBit(WeaselFlag::CLEAR_DOWN);
			SetBit(WeaselFlag::CLEAR_FLAG);
			return S_OK;
		}
	}
	_ProcessKeyEvent(wParam, lParam, pfEaten);
	if (*pfEaten)
	{
		_fTestKeyDownPending = true;
	}
	return S_OK;
}

STDAPI WeaselTSF::OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
#ifdef TEST
	LOG(INFO) << std::format("From OnKeyDown. _fTestKeyDownPending = {}, wParam = {:#x}, lParam = {:#x}, pContext = {:#x}", _fTestKeyDownPending, wParam, lParam, (size_t)pContext);
#endif // TEST
	_fTestKeyUpPending = false;
	if (_fTestKeyDownPending)
	{
		_fTestKeyDownPending = false;
		*pfEaten = TRUE;
	}
	else
	{
		_ProcessKeyEvent(wParam, lParam, pfEaten);
#ifdef TEST
		LOG(INFO) << std::format("From OnKeyDown. *pfEaten = {}", *pfEaten);
#endif // TEST
	}
	_UpdateComposition(pContext);
	return S_OK;
}

STDAPI WeaselTSF::OnTestKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
#ifdef TEST
	LOG(INFO) << std::format("From OnTestKeyUp. wParam = {:#x}, lParam = {:#x}, pContext = {:#x}", wParam, (unsigned)lParam, (size_t)pContext);
#endif // TEST
	_fTestKeyDownPending = false;
	if (_fTestKeyUpPending)
	{
		*pfEaten = TRUE;
		return S_OK;
	}
	if (GetBit(WeaselFlag::GAME_MODE) && wParam == 0x0C)
	{
		ResetBit(WeaselFlag::CLEAR_DOWN);
		_UpdateComposition(pContext);
		return S_OK;
	}
	_ProcessKeyEvent(wParam, lParam, pfEaten);
	_UpdateComposition(pContext);
	if (*pfEaten)
	{
		_fTestKeyUpPending = true;
	}
	return S_OK;
}

STDAPI WeaselTSF::OnKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
#ifdef TEST
	LOG(INFO) << std::format("From OnKeyUp. wParam = {:#x}, lParam = {:#x}", wParam, (unsigned)lParam);
#endif // TEST
	_fTestKeyDownPending = false;
	if (_fTestKeyUpPending)
	{
		_fTestKeyUpPending = false;
		*pfEaten = TRUE;
	}
	else
	{
		_ProcessKeyEvent(wParam, lParam, pfEaten);
#ifdef TEST
		LOG(INFO) << std::format("From OnKeyUp. *pfEaten = {}", *pfEaten);
#endif // TEST
		if (!GetBit(WeaselFlag::ASYNC_EDIT))
			_UpdateComposition(pContext);
	}

	return S_OK;
}

STDAPI WeaselTSF::OnPreservedKey(ITfContext* pContext, REFGUID rguid, BOOL* pfEaten)
{
	*pfEaten = FALSE;
	if (IsEqualGUID(rguid, WEASEL_UILESS_MODE_PRESERVED_KEY))
	{
		_bitset.flip(static_cast<int>(WeaselFlag::GAME_MODE));
	}
	else if (IsEqualGUID(rguid, WEASEL_CARET_FOLLOWING_PRESERVED_KEY))
	{
		_bitset.flip(static_cast<int>(WeaselFlag::CARET_FOLLOWING));
		_cand->SetCaretFollowing(GetBit(WeaselFlag::CARET_FOLLOWING));
	}
	else if (IsEqualGUID(rguid, WEASEL_DAEMON_PRESERVED_KEY))
	{
		_bitset.flip(static_cast<int>(WeaselFlag::DAEMON_ENABLE));
		if (_pGlobalCompartmentDaemon)
		{
			VARIANT var{};
			var.vt = VT_I4;
			if (GetBit(WeaselFlag::DAEMON_ENABLE))
			{
				var.lVal = 0xFC01;
			}
			else
			{
				var.lVal = 0xFC00;
			}
			_pGlobalCompartmentDaemon->SetValue(_tfClientId, &var);
		}
	}
	return S_OK;
}

BOOL WeaselTSF::_InitKeyEventSink()
{
	com_ptr<ITfKeystrokeMgr> pKeystrokeMgr;
	HRESULT hr;

	if (_pThreadMgr->QueryInterface(&pKeystrokeMgr) != S_OK)
		return FALSE;

	hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink*)this, TRUE);

	return (hr == S_OK);
}

void WeaselTSF::_UninitKeyEventSink()
{
	com_ptr<ITfKeystrokeMgr> pKeystrokeMgr;

	if (_pThreadMgr->QueryInterface(&pKeystrokeMgr) != S_OK)
		return;

	pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);
}

BOOL WeaselTSF::_InitPreservedKey()
{
	com_ptr<ITfKeystrokeMgr> pKeystrokeMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(&pKeystrokeMgr)))
	{
		_preservedKeyGameMode.uVKey = 0x47;			// 'G'
		_preservedKeyGameMode.uModifiers = TF_MOD_CONTROL | TF_MOD_SHIFT;

		_preservedKeyCaretFollowing.uVKey = 0x46;	// 'F'
		_preservedKeyCaretFollowing.uModifiers = TF_MOD_CONTROL | TF_MOD_SHIFT;

		_preservedKeyDaemon.uVKey = 0x44;			// 'D'
		_preservedKeyDaemon.uModifiers = TF_MOD_CONTROL | TF_MOD_SHIFT;

		std::wstring uilessMode{ L"сно╥дёй╫" };
		auto hr = pKeystrokeMgr->PreserveKey(_tfClientId, WEASEL_UILESS_MODE_PRESERVED_KEY, &_preservedKeyGameMode, uilessMode.data(), uilessMode.size());
		hr = pKeystrokeMgr->PreserveKey(_tfClientId, WEASEL_CARET_FOLLOWING_PRESERVED_KEY, &_preservedKeyCaretFollowing, L"", 0);
		hr = pKeystrokeMgr->PreserveKey(_tfClientId, WEASEL_DAEMON_PRESERVED_KEY, &_preservedKeyDaemon, L"", 0);

		return SUCCEEDED(hr);
	}
	return TRUE;
#if 0
	com_ptr<ITfKeystrokeMgr> pKeystrokeMgr;
	if (_pThreadMgr->QueryInterface(pKeystrokeMgr.GetAddressOf()) != S_OK)
	{
		return FALSE;
	}
	TF_PRESERVEDKEY preservedKeyImeMode;

	/* Define SHIFT ONLY for now */
	preservedKeyImeMode.uVKey = VK_SHIFT;
	preservedKeyImeMode.uModifiers = TF_MOD_ON_KEYUP;

	auto hr = pKeystrokeMgr->PreserveKey(
		_tfClientId,
		GUID_IME_MODE_PRESERVED_KEY,
		&preservedKeyImeMode, L"", 0);

	return SUCCEEDED(hr);
#endif
}

void WeaselTSF::_UninitPreservedKey()
{
	com_ptr<ITfKeystrokeMgr> pKeystrokeMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(&pKeystrokeMgr)))
	{
		pKeystrokeMgr->UnpreserveKey(WEASEL_UILESS_MODE_PRESERVED_KEY, &_preservedKeyGameMode);
		pKeystrokeMgr->UnpreserveKey(WEASEL_CARET_FOLLOWING_PRESERVED_KEY, &_preservedKeyCaretFollowing);
		pKeystrokeMgr->UnpreserveKey(WEASEL_DAEMON_PRESERVED_KEY, &_preservedKeyDaemon);
	}
}

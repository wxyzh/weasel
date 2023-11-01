module;
#include "stdafx.h"
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
				SetBit(16);		// _bitset[16]: _CompositionWithCapsLock
			}
			break;

		case 0x4002'FFE5:		// Caps Lock up
			if (GetBit(16))		// _bitset[16]: _CompositionWithCapsLock
			{
				ReSetBit(16);
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
			SetBit(18);
			_inputKey = ke.keycode;
			*pfEaten = true;
		}
#ifdef TEST
		LOG(INFO) << std::format("From WeaselTSF::_ProcessKeyEvent. ke = 0x{:X}, Caps_Lock = {}", (unsigned)ke, GetBit(16));
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
	ReSetBit(13);		// _bitset[13]: _FistKeyComposition
	if (_fTestKeyDownPending)
	{
		*pfEaten = TRUE;
		return S_OK;
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
		if (!GetBit(15))	// _bitset[15]: _AsyncEdit
			_UpdateComposition(pContext);
	}

	return S_OK;
}

STDAPI WeaselTSF::OnPreservedKey(ITfContext* pContext, REFGUID rguid, BOOL* pfEaten)
{
	*pfEaten = FALSE;
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
}

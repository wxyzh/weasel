module;
#include "stdafx.h"
#include "Globals.h"
module WeaselTSF;
import WeaselIPC;
import KeyEvent;
import CandidateList;

void WeaselTSF::_ProcessKeyEvent(WPARAM wParam, LPARAM lParam, BOOL* pfEaten, bool isTest)
{
	if (!_IsKeyboardOpen())
	{
		/*if (wParam == 0x39 && GetKeyState(VK_CONTROL) < 0)
		{
			_SetKeyboardOpen(TRUE);
		}*/
		return;
	}

	ResetBit(WeaselFlag::FIRST_KEY_COMPOSITION);

	if (GetBit(WeaselFlag::COMPOSITION_WITH_CAPSLOCK) && wParam == VK_CAPITAL && GetKeyState(VK_CAPITAL) > 0)
	{
		ResetBit(WeaselFlag::COMPOSITION_WITH_CAPSLOCK);
		SimulatingKeyboardEvents(VK_CAPITAL);
	}

	if (_IsComposing() && wParam == VK_CAPITAL && GetKeyState(VK_CAPITAL) < 0)
	{
		SetBit(WeaselFlag::COMPOSITION_WITH_CAPSLOCK);
	}	

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
		if (!isTest && !(*pfEaten) && !GetBit(WeaselFlag::ASCII_PUNCT) && !(lParam >> 31) && ispunct(ke.keycode) && !(ke.mask & (ibus::CONTROL_MASK | ibus::ALT_MASK)))
		{
			SetBit(WeaselFlag::ASYNC_DIGIT_PUNCT_EATEN);
			_keycode = ke.keycode;
			*pfEaten = true;
		}
	}
}

STDAPI WeaselTSF::OnSetFocus(BOOL fForeground)
{
	if (fForeground)
	{
		m_client.FocusIn();
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
	_fTestKeyUpPending = false;
	if (_fTestKeyDownPending)
	{
		*pfEaten = TRUE;
		return S_OK;
	}	
	_ProcessKeyEvent(wParam, lParam, pfEaten, true);
	if (*pfEaten)
	{
		_fTestKeyDownPending = true;
	}
	return S_OK;
}

STDAPI WeaselTSF::OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
	_lastKey = static_cast<unsigned short>(wParam);
	_fTestKeyUpPending = false;
	if (_fTestKeyDownPending)
	{
		_fTestKeyDownPending = false;
		*pfEaten = TRUE;
	}
	else
	{
		_ProcessKeyEvent(wParam, lParam, pfEaten);
	}
	if (GetBit(WeaselFlag::AUTOCAD))
	{
		if (lParam == 0x4000'0000 || lParam == 0x4000'0001)		
		{
			if (!GetBit(WeaselFlag::INLINE_PREEDIT))
			{
				SetBit(WeaselFlag::NOT_INLINE_PREEDIT_LOST_FIRST_KEY);
			}
			else
			{
				SetBit(WeaselFlag::INLINE_PREEDIT_LOST_FIRST_KEY);
			}
		}
	}
	_UpdateComposition(pContext);

	return S_OK;
}

STDAPI WeaselTSF::OnTestKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
	_fTestKeyDownPending = false;
	if (_fTestKeyUpPending)
	{
		*pfEaten = TRUE;
		return S_OK;
	}
	_ProcessKeyEvent(wParam, lParam, pfEaten, true);
	_UpdateComposition(pContext);
	if (*pfEaten)
	{
		_fTestKeyUpPending = true;
	}
	return S_OK;
}

STDAPI WeaselTSF::OnKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
	_fTestKeyDownPending = false;
	if (_fTestKeyUpPending)
	{
		_fTestKeyUpPending = false;
		*pfEaten = TRUE;
	}
	else
	{
		_ProcessKeyEvent(wParam, lParam, pfEaten);
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
		if (GetBit(WeaselFlag::GAME_MODE))
		{
			SetBit(WeaselFlag::GAME_MODE_SELF_REDRAW);
		}
		else
		{
			ResetBit(WeaselFlag::GAME_MODE_SELF_REDRAW);
		}
		Flip(WeaselFlag::GAME_MODE);
		*pfEaten = TRUE;
	}
	else if (IsEqualGUID(rguid, WEASEL_CARET_FOLLOWING_PRESERVED_KEY))
	{
		Flip(WeaselFlag::CARET_FOLLOWING);
		_cand->SetCaretFollowing(GetBit(WeaselFlag::CARET_FOLLOWING));
		*pfEaten = TRUE;
	}
	else if (IsEqualGUID(rguid, WEASEL_DAEMON_PRESERVED_KEY))
	{
		Flip(WeaselFlag::DAEMON_ENABLE);
		if (_pGlobalCompartment)
		{
			UpdateGlobalCompartment(true);
		}
		*pfEaten = TRUE;
	}
	/*else if (IsEqualGUID(rguid, GUID_IME_MODE_PRESERVED_KEY))
	{
		com_ptr<ITfInputProcessorProfileMgr> pInputProcessorProfileMgr;
		if (SUCCEEDED(pInputProcessorProfileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_ALL)))
		{
			com_ptr<ITfInputProcessorProfileSubstituteLayout> pInputProcessorProfileSubstituteLayout;
			if (SUCCEEDED(pInputProcessorProfileMgr->QueryInterface(&pInputProcessorProfileSubstituteLayout)))
			{
				HKL hKL{ nullptr }, oldHKL{};
				HRESULT hr = pInputProcessorProfileSubstituteLayout->GetSubstituteKeyboardLayout(c_clsidTextService, TEXTSERVICE_LANGID, c_guidProfile,	&hKL);
				if (SUCCEEDED(hr) && hKL)
				{					
					oldHKL = ActivateKeyboardLayout(hKL, KLF_SETFORPROCESS);
					if (oldHKL)
					{
						StoreTextServiceHandle(oldHKL);
						*pfEaten = TRUE;
					}
				}
			}
		}
	}*/
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

		//_preservedKeyImeMode.uVKey = 0x39;			// '9'
		//_preservedKeyImeMode.uModifiers = TF_MOD_CONTROL;

		std::wstring uilessMode{ L"сно╥дёй╫" };
		auto hr = pKeystrokeMgr->PreserveKey(_tfClientId, WEASEL_UILESS_MODE_PRESERVED_KEY, &_preservedKeyGameMode, uilessMode.data(), uilessMode.size());
		hr = pKeystrokeMgr->PreserveKey(_tfClientId, WEASEL_CARET_FOLLOWING_PRESERVED_KEY, &_preservedKeyCaretFollowing, L"", 0);
		hr = pKeystrokeMgr->PreserveKey(_tfClientId, WEASEL_DAEMON_PRESERVED_KEY, &_preservedKeyDaemon, L"", 0);
		// hr = pKeystrokeMgr->PreserveKey(_tfClientId, GUID_IME_MODE_PRESERVED_KEY, &_preservedKeyImeMode, L"", 0);

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
		// pKeystrokeMgr->UnpreserveKey(GUID_IME_MODE_PRESERVED_KEY, &_preservedKeyImeMode);
	}
}

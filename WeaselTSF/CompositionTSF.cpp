module;
#include "stdafx.h"
#include "test.h"
#ifdef TEST
#ifdef _M_X64
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif
#endif // TEST
module WeaselTSF;
import Composition;
import CandidateList;
import WeaselUtility;

void WeaselTSF::_StartComposition(ITfContext* pContext, BOOL fCUASWorkaroundEnabled)
{
	com_ptr<CStartCompositionEditSession> pStartCompositionEditSession;
	pStartCompositionEditSession.Attach(new CStartCompositionEditSession(this, pContext, fCUASWorkaroundEnabled));
	_cand->StartUI();
	if (pStartCompositionEditSession != nullptr)
	{
		HRESULT hr;
		auto ret = pContext->RequestEditSession(_tfClientId, pStartCompositionEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
		SetBit(6);						// _bitset[6]:	_BeginComposition
		SetBit(13);						// _bitset[13]: _FistKeyComposition
#ifdef TEST
#ifdef _M_X64
			LOG(INFO) << std::format("From _StartComposition. hr = {:#x}, ret = {:#x}", (unsigned)hr, (unsigned)ret);
#endif // _M_X64
#endif // TEST
	}
}

void WeaselTSF::_EndComposition(ITfContext* pContext, BOOL clear)
{
	com_ptr<CEndCompositionEditSession> pEditSession;
	pEditSession.Attach(new CEndCompositionEditSession(this, pContext, _pComposition, clear));
	HRESULT hr;

	if (pEditSession != nullptr)
	{
		pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
	}
	// after pEditSession, less flicker
	_cand->EndUI();
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From _EndComposition. hr = {:#x}", (unsigned)hr);
#endif // _M_X64
#endif // TEST
}

/* Composition Window Handling */
BOOL WeaselTSF::_UpdateCompositionWindow(ITfContext* pContext)
{
	com_ptr<ITfContextView> pContextView;
	HRESULT hr;

	hr = pContext->GetActiveView(&pContextView);
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From _UpdateCompositionWindow. hr = {:#x}", (unsigned)hr);
#endif // _M_X64
#endif // TEST
	if (FAILED(hr))
		return FALSE;
	com_ptr<CGetTextExtentEditSession> pEditSession;
	pEditSession.Attach(new CGetTextExtentEditSession(this, pContext, pContextView, _pComposition));
	if (pEditSession == NULL)
	{
		return FALSE;
	}
	
	auto ret = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READ, &hr);
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From _UpdateCompositionWindow. hr = {:#x}, ret = {:#x}", (unsigned)hr, (unsigned)ret);
#endif // _M_X64
#endif // TEST
	return SUCCEEDED(ret);
}

void WeaselTSF::_SetCompositionPosition(const RECT& rc)
{
	/* Test if rect is valid.
	 * If it is invalid during CUAS test, we need to apply CUAS workaround */	
	if (!_fCUASWorkaroundTested)
	{
		RECT rect{};
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		_fCUASWorkaroundTested = true;
		if ((abs(rc.left - rect.left) < 2 && abs(rc.top - rect.top) < 2)
			|| (abs(rc.left - rect.right) < 5 && abs(rc.top - rect.bottom) < 5))
		{
			_fCUASWorkaroundEnabled = true;
#ifdef TEST
#ifdef _M_X64
			LOG(INFO) << std::format("From _SetCompositionPosition. _fCUASWorkaroundEnabled = {}, CUASWorkaroundMode = {}", _fCUASWorkaroundEnabled, GetBit(9));
#endif // _M_X64
#endif // TEST
		}
	}

#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From _SetCompositionPosition. left = {}, top = {}, right = {}, bottom = {}", rc.left, rc.top, rc.right, rc.bottom);
#endif // _M_X64
#endif // TEST

	m_client.UpdateInputPosition(rc);
	_cand->UpdateInputPosition(rc);	
}

BOOL WeaselTSF::_ShowInlinePreedit(ITfContext* pContext, const std::shared_ptr<weasel::Context> context)
{
	com_ptr<CInlinePreeditEditSession> pEditSession;
	pEditSession.Attach(new CInlinePreeditEditSession(this, pContext, _pComposition, context));
	if (pEditSession != nullptr)
	{
		HRESULT hr;
		pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
	}

	return TRUE;
}

BOOL WeaselTSF::_InsertText(ITfContext* pContext, std::wstring_view text)
{
	com_ptr<CInsertTextEditSession> pEditSession;
	pEditSession.Attach(new CInsertTextEditSession(this, pContext, _pComposition, text));
	HRESULT hr;

#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From _InsertText. text = {}", to_string(text.data(), CP_UTF8));
#endif // _M_X64
#endif // TEST

	if (pEditSession != nullptr)
	{
		pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
	}

	return TRUE;
}

void WeaselTSF::_UpdateComposition(ITfContext* pContext)
{
	HRESULT hr;
	_pEditSessionContext = pContext;
	_pEditSessionContext->RequestEditSession(_tfClientId, this, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From _UpdateComposition. hr = {:#x}, pContext = {:#x}, _tfClientId = {:#x}", (unsigned)hr, (size_t)pContext, _tfClientId);
#endif // _M_X64
#endif // TEST
}

/* Composition State */
STDAPI WeaselTSF::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition* pComposition)
{
	// NOTE:
	// This will be called when an edit session ended up with an empty composition string,
	// Even if it is closed normally.
	// Silly M$.
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From OnCompositionTerminated. _AbortComposition. _AutoCADTest = {}", GetBit(9));
#endif // _M_X64
#endif // TEST
	static int count{};
	++count;
	SetBit(9);			// _bitset[9]:  _AutoCAD
	if (count == 2)
	{
		count = 0;
		SetBit(10);		// _bitset[10]: _DynamicInput
	}
	_AbortComposition();

	return S_OK;
}

void WeaselTSF::_AbortComposition(bool clear)
{
	if (GetBit(9))							// _bitset[9]:  _AutoCAD
	{
		ReSetBit(9);						// _bitset[9]:  _AutoCAD
		if (GetBit(10) || GetBit(11))		// _bitset[10]: _DynamicInput, _bitset[11]: _NonDynamicInput
		{			
			ReSetBit(10);					// _bitset[10]: _DynamicInput
			ReSetBit(11);					// _bitset[11]: _NonDynamicInput
			BOOL eaten;
			_ProcessKeyEvent(0x8, 0xE001, &eaten);
		}
	}
	else
	{
		m_client.ClearComposition();
	}

	if (_IsComposing()) {
		_EndComposition(_pEditSessionContext, clear);
	}
	_cand->Destroy();
}

void WeaselTSF::_FinalizeComposition()
{
	_pComposition = nullptr;
}

void WeaselTSF::_SetComposition(ITfComposition* pComposition)
{
	_pComposition = pComposition;
}

BOOL WeaselTSF::_IsComposing()
{
	return _pComposition != NULL;
}
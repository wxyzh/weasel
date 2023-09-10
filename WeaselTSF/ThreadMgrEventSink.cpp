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
import CandidateList;

STDAPI WeaselTSF::OnInitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::OnInitDocumentMgr. pDocMgr = {:#x}", (size_t)pDocMgr);
#endif // _M_X64
#endif // TEST
	return S_OK;
}

STDAPI WeaselTSF::OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::OnUninitDocumentMgr. pDocMgr = {:#x}", (size_t)pDocMgr);
#endif // _M_X64
#endif // TEST
	return S_OK;
}

STDAPI WeaselTSF::OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus)
{
	auto ret = _InitTextEditSink(pDocMgrFocus);

#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::OnSetFocus. pDocMgrFocus = {:#x}, pDocMrgPrevFocus = {:#x}, ret = {}", (size_t)pDocMgrFocus, (size_t)pDocMgrPrevFocus, ret);
#endif // _M_X64
#endif // TEST

	if (!pDocMgrFocus)
	{
		SetBit(7);			// _bitset[7]: _FocusChanged
	}
	else if(GetBit(14))		// _bitset[14]: _KeyboardDisabled
	{
		_SetKeyboardOpen(TRUE);
	}

	com_ptr<ITfDocumentMgr> pCandidateListDocumentMgr;
	com_ptr<ITfContext> pTfContext = _GetUIContextDocument();
	if ((nullptr != pTfContext) && SUCCEEDED(pTfContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
	{
		if (pCandidateListDocumentMgr != pDocMgrFocus)
		{
			_cand->OnKillThreadFocus();
		}
		else
		{
			_cand->OnSetThreadFocus();
		}		
	}

	_pDocMgrLastFocused = pDocMgrFocus;

	return S_OK;
}

STDAPI WeaselTSF::OnPushContext(ITfContext *pContext)
{
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::OnPushContext. pContext = {:#x}", (size_t)pContext);
#endif // _M_X64
#endif // TEST
	return S_OK;
}

STDAPI WeaselTSF::OnPopContext(ITfContext *pContext)
{
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::OnPopContext. pContext = {:#x}", (size_t)pContext);
#endif // _M_X64
#endif // TEST
	return S_OK;
}

BOOL WeaselTSF::_InitThreadMgrEventSink()
{
	com_ptr<ITfSource> pSource;
	if (_pThreadMgr->QueryInterface(&pSource) != S_OK)
		return FALSE;
	if (pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink *) this, &_dwThreadMgrEventSinkCookie) != S_OK)
	{
		_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
		return FALSE;
	}
	return TRUE;
}

void WeaselTSF::_UninitThreadMgrEventSink()
{
	com_ptr<ITfSource> pSource;
	if (_dwThreadMgrEventSinkCookie == TF_INVALID_COOKIE)
		return;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(&pSource)))
	{
		pSource->UnadviseSink(_dwThreadMgrEventSinkCookie);
	}
	_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
}

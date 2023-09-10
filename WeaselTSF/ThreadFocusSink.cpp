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

STDMETHODIMP WeaselTSF::OnSetThreadFocus()
{
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::OnSetThreadFocus.");
#endif // _M_X64
#endif // TEST
	if (_cand)
	{
		com_ptr<ITfDocumentMgr> pCandidateListDocumentMgr;
		
		if (SUCCEEDED(_pTextEditSinkContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
		{
			if (pCandidateListDocumentMgr == _pDocMgrLastFocused)
			{
				_cand->OnSetThreadFocus();
			}
		}
	}

	return S_OK;
}
STDMETHODIMP WeaselTSF::OnKillThreadFocus()
{
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::OnKillThreadFocus.");
#endif // _M_X64
#endif // TEST
	if (_cand)
	{
		com_ptr<ITfDocumentMgr> pCandidateListDocumentMgr;

		if (SUCCEEDED(_pTextEditSinkContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
		{
			_pDocMgrLastFocused = pCandidateListDocumentMgr;
			_cand->OnKillThreadFocus();
		}
	}
	_AbortComposition();
	return S_OK;
}

BOOL WeaselTSF::_InitThreadFocusSink()
{
	com_ptr<ITfSource> pSource;

	if (FAILED(_pThreadMgr->QueryInterface(&pSource)))
	{
		return FALSE;
	}

	if (FAILED(pSource->AdviseSink(IID_ITfThreadFocusSink, (ITfThreadFocusSink*)this, &_dwThreadFocusSinkCookie)))
	{
		return FALSE;
	}

	return TRUE;
}

void WeaselTSF::_UninitThreadFocusSink()
{
	com_ptr<ITfSource> pSource;

	if (FAILED(_pThreadMgr->QueryInterface(&pSource)))
	{
		return;
	}

	if (FAILED(pSource->UnadviseSink(_dwThreadFocusSinkCookie)))
	{
		return;
	}
}
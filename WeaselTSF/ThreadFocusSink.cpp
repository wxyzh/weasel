module;
#include "stdafx.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;
import CandidateList;

STDMETHODIMP WeaselTSF::OnSetThreadFocus()
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnSetThreadFocus. _pComposition ? {:s}", (bool)_IsComposing());
#endif // TEST
	if (_pComposition)
	{
		com_ptr<ITfDocumentMgr> pCandidateListDocumentMgr;
		if (SUCCEEDED(_pTextEditSinkContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
		{
			if (pCandidateListDocumentMgr == _pDocMgrLastFocused)
			{
				_cand->SetThreadFocus();
			}
		}
	}
	return S_OK;
}
STDMETHODIMP WeaselTSF::OnKillThreadFocus()
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnKillThreadFocus. _pComposition ? {:s}", (bool)_IsComposing());
#endif // TEST
	if (_pComposition)
	{
		com_ptr<ITfDocumentMgr> pCandidateListDocumentMgr;

		if (SUCCEEDED(_pTextEditSinkContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
		{
			if (_pDocMgrLastFocused)
			{
				_pDocMgrLastFocused.Release();
			}
			_pDocMgrLastFocused = pCandidateListDocumentMgr;
		}
		_cand->KillThreadFocus();
	}
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
module;
#include "stdafx.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;
import CandidateList;

STDAPI WeaselTSF::OnInitDocumentMgr(ITfDocumentMgr* pDocMgr)
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnInitDocumentMgr. pDocMgr = {:#x}", (size_t)pDocMgr);
#endif // TEST
	return SwitchToActiveContext();
}

STDAPI WeaselTSF::OnUninitDocumentMgr(ITfDocumentMgr* pDocMgr)
{
	return SwitchToActiveContext();
}

STDAPI WeaselTSF::OnSetFocus(ITfDocumentMgr* pDocMgrFocus, ITfDocumentMgr* pDocMgrPrevFocus)
{
	_InitTextEditSink(pDocMgrFocus);

#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnSetFocus. pDocMgrFocus = {:#x}, pDocMrgPrevFocus = {:#x}", (size_t)pDocMgrFocus, (size_t)pDocMgrPrevFocus);
#endif // TEST

	if (!pDocMgrFocus)
	{
		SetBit(WeaselFlag::FOCUS_CHANGED);
	}
	else if (GetBit(WeaselFlag::KEYBOARD_DISABLE))
	{
		_SetKeyboardOpen(TRUE);
	}

	com_ptr<ITfDocumentMgr> pCandidateListDocumentMgr;
	com_ptr<ITfContext> pTfContext = _GetUIContextDocument();
	if ((nullptr != pTfContext) && SUCCEEDED(pTfContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
	{
		if (pCandidateListDocumentMgr != pDocMgrFocus)
		{
			_cand->KillThreadFocus();
		}
		else
		{
			_cand->SetThreadFocus();
		}
	}

	_pDocMgrLastFocused = pDocMgrFocus;

	return S_OK;
}

STDAPI WeaselTSF::OnPushContext(ITfContext* pContext)
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnPushContext. pContext = {:#x}", (size_t)pContext);
#endif // TEST
	return SwitchToActiveContext();
}

STDAPI WeaselTSF::OnPopContext(ITfContext* pContext)
{
	return SwitchToActiveContext();
}

HRESULT WeaselTSF::SwitchContext(ITfContext* pContext)
{
	if (_pTextEditSinkContext == pContext && _threadMgrEventSinkInitialized)
		return S_OK;

	_pTextEditSinkContext = pContext;
	return S_OK;
}

HRESULT WeaselTSF::SwitchToActiveContext()
{
	com_ptr<ITfDocumentMgr> pDocumentMgr;
	if (FAILED(_pThreadMgr->GetFocus(&pDocumentMgr)))
	{
		return SwitchContext(nullptr);
	}
	return SwitchToActiveContextForDocumentManager(pDocumentMgr);
}

HRESULT WeaselTSF::SwitchToActiveContextForDocumentManager(ITfDocumentMgr* pDocumentMgr)
{
	if (!pDocumentMgr)
		return SwitchContext(nullptr);

	com_ptr<ITfContext> pContext;
	if (FAILED(pDocumentMgr->GetTop(&pContext)) || !pContext)
	{
		return SwitchContext(nullptr);
	}

	return SwitchContext(pContext);
}

BOOL WeaselTSF::_InitThreadMgrEventSink()
{
	com_ptr<ITfSource> pSource;
	if (_pThreadMgr->QueryInterface(&pSource) != S_OK)
		return FALSE;
	if (pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink*)this, &_dwThreadMgrEventSinkCookie) != S_OK)
	{
		_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
		return FALSE;
	}
	_threadMgrEventSinkInitialized = true;
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
	_threadMgrEventSinkInitialized = false;
}

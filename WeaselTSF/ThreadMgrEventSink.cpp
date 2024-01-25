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
	return E_NOTIMPL;
}

STDAPI WeaselTSF::OnUninitDocumentMgr(ITfDocumentMgr* pDocMgr)
{
	return E_NOTIMPL;
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
	return E_NOTIMPL;
}

STDAPI WeaselTSF::OnPopContext(ITfContext* pContext)
{
	return E_NOTIMPL;
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

//void WeaselTSF::EnsurePrivateContextExists(ITfContext* pContext)
//{
//	if (pContext == nullptr)
//	{
//		// Do not care about nullptr pContext.
//		return;
//	}
//	if (m_privateContext.contains(pContext))
//	{
//		return;
//	}
//
//	com_ptr<ITfSource> pSource;
//	if (SUCCEEDED(pContext->QueryInterface(&pSource)) && pSource)
//	{
//		DWORD textEditSinkCookie{ TF_INVALID_COOKIE };
//		DWORD textLayoutSinkCookie{ TF_INVALID_COOKIE };
//		if (FAILED(pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*)this, &textEditSinkCookie)))
//		{
//			// In general this should not happen
//			textEditSinkCookie = TF_INVALID_COOKIE;
//		}
//		if (FAILED(pSource->AdviseSink(IID_ITfTextLayoutSink, (ITfTextLayoutSink*)this, &textLayoutSinkCookie)))
//		{
//			// In general this should not happen.
//			textEditSinkCookie = TF_INVALID_COOKIE;
//		}
//
//		// Register private context with sink-cleanup callback.
//		m_privateContext.emplace(pContext, std::make_unique<PrivateContextWrapper>
//			([=]()
//			{
//				if (textEditSinkCookie != TF_INVALID_COOKIE)
//				{
//					pSource->UnadviseSink(textEditSinkCookie);
//				}
//				if (textLayoutSinkCookie != TF_INVALID_COOKIE)
//				{
//					pSource->UnadviseSink(textLayoutSinkCookie);
//				}
//			}));
//	}
//}
//
//void WeaselTSF::RemovePrivateContextIfExists(ITfContext* pContext)
//{
//	m_privateContext.erase(pContext);
//}
//
//void WeaselTSF::UninitPrivateContexts()
//{ 
//	m_privateContext.clear(); 
//}
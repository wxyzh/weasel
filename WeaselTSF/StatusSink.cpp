module;
#include "stdafx.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;

HRESULT WeaselTSF::OnStatusChange(__in ITfContext* pContext, __in DWORD dwFlags)
{
#ifdef TEST
	LOG(INFO) << std::format("WeaselTSF::OnStatusChange. dwFlags = {:#x}", dwFlags);
#endif // TEST
	return S_OK;
}

BOOL WeaselTSF::_InitStatusSink()
{
	com_ptr<ITfSource> pSource;

	if (FAILED(_pThreadMgr->QueryInterface(&pSource)))
	{
		return FALSE;
	}

	if (FAILED(pSource->AdviseSink(IID_ITfStatusSink, (ITfStatusSink*)this, &_dwStatusSinkCookie)))
	{
		return FALSE;
	}

	return TRUE;
}

void WeaselTSF::_UninitStatusSink()
{
	com_ptr<ITfSource> pSource;

	if (FAILED(_pThreadMgr->QueryInterface(&pSource)))
	{
		return;
	}

	if (FAILED(pSource->UnadviseSink(_dwStatusSinkCookie)))
	{
		return;
	}
}
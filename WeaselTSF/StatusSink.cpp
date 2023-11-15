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
#ifdef TEST
		LOG(INFO) << std::format("WeaselTSF::_InitStatusSink. First.");
#endif // TEST
		return FALSE;
	}
	E_INVALIDARG;
	CONNECT_E_CANNOTCONNECT;
	auto hr = pSource->AdviseSink(IID_ITfStatusSink, (ITfStatusSink*)this, &_dwStatusSinkCookie);
	if (FAILED(hr))
	{
#ifdef TEST
		LOG(INFO) << std::format("WeaselTSF::_InitStatusSink. Second. hr = 0x{:X}", (unsigned)hr);
#endif // TEST
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
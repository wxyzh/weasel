module;
#include "stdafx.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;

STDMETHODIMP WeaselTSF::OnStartCleanupContext()
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnStartCleanupContext.");
#endif // TEST
	return S_OK;
}

STDMETHODIMP WeaselTSF::OnEndCleanupContext()
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnEndCleanupContext.");
#endif // TEST
	return S_OK;
}

BOOL WeaselTSF::_InitCleanupContextDurationSink()
{
	HRESULT hr{E_FAIL};
	com_ptr<ITfSourceSingle> pSourceSingle;

	if (SUCCEEDED(_pThreadMgr->QueryInterface(&pSourceSingle)))
	{
		hr = pSourceSingle->AdviseSingleSink(_tfClientId, IID_ITfCleanupContextDurationSink, (ITfCleanupContextDurationSink*)this);
	}
	return SUCCEEDED(hr);
}

void WeaselTSF::_UninitCleanupContextDurationSink()
{
	com_ptr<ITfSourceSingle> pSourceSingle;

	if (SUCCEEDED(_pThreadMgr->QueryInterface(&pSourceSingle)))
	{
		pSourceSingle->UnadviseSingleSink(_tfClientId, IID_ITfCleanupContextDurationSink);
	}
}
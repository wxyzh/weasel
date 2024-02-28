module;
#include "stdafx.h"
module WeaselTSF;

STDMETHODIMP WeaselTSF::OnStartCleanupContext()
{
	return S_OK;
}

STDMETHODIMP WeaselTSF::OnEndCleanupContext()
{
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
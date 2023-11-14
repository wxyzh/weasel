module;
#include "stdafx.h"
#include "Globals.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;

STDMETHODIMP WeaselTSF::OnKeyTraceDown(WPARAM wParam, LPARAM lParam)
{
	return S_OK;
}

STDMETHODIMP WeaselTSF::OnKeyTraceUp(WPARAM wParam, LPARAM lParam)
{
	return S_OK;
}

BOOL WeaselTSF::_InitKeyTraceEventSink()
{
	com_ptr<ITfSource> pSource;
	HRESULT hr;

	if (SUCCEEDED(hr = _pThreadMgr->QueryInterface(&pSource)))
	{
		hr = pSource->AdviseSink(IID_ITfKeyTraceEventSink, (ITfKeyTraceEventSink*)this, &_dwKeyTraceEventSinkCookie);
	}

	return SUCCEEDED(hr);
}

void WeaselTSF::_UninitKeyTraceEventSink()
{
	com_ptr<ITfSource> pSource;

	if (SUCCEEDED(_pThreadMgr->QueryInterface(&pSource)))
	{
		pSource->UnadviseSink(_dwKeyTraceEventSinkCookie);
		_dwKeyTraceEventSinkCookie = TF_INVALID_COOKIE;
	}
}
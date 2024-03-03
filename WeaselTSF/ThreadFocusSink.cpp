module;
#include "stdafx.h"
#include "test.h"
module WeaselTSF;
import CandidateList;
import ResponseParser;

STDMETHODIMP WeaselTSF::OnSetThreadFocus()
{
#ifdef TEST
	std::wstring buffer = std::format(L"From WeaselTSF::OnSetThreadFocus(). ascii_mode = {:s}\n", _status.ascii_mode);
	WriteConsole(buffer);
#endif // TEST
	if (m_client.Echo())
	{
		m_client.ProcessKeyEvent(0);
		_UpdateComposition(_pEditSessionContext);
	}
#ifdef TEST
	buffer = std::format(L"From WeaselTSF::OnSetThreadFocus(). ascii_mode = {:s}\n", _status.ascii_mode);
	WriteConsole(buffer);
#endif // TEST

	return S_OK;
}

STDMETHODIMP WeaselTSF::OnKillThreadFocus()
{
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
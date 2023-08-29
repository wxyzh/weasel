module;
#include "stdafx.h"
module WeaselTSF;

BOOL WeaselTSF::_InitActiveLanguageProfileNotifySink()
{
	com_ptr<ITfSource> pSource;
	BOOL ret = FALSE;

	if (_pThreadMgr->QueryInterface(&pSource) != S_OK)
	{
		return ret;
	}

	if (pSource->AdviseSink(IID_ITfActiveLanguageProfileNotifySink, (ITfActiveLanguageProfileNotifySink*)this, &_activeLanguageProfileNotifySinkCookie) != S_OK)
	{
		_activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
		goto Exit;
	}

	ret = TRUE;

Exit:
	return ret;
}

void WeaselTSF::_UninitActiveLanguageProfileNotifySink()
{
	com_ptr<ITfSource> pSource;

	if (_activeLanguageProfileNotifySinkCookie == TF_INVALID_COOKIE)
	{
		return;
	}

	if (_pThreadMgr->QueryInterface(&pSource) == S_OK)
	{
		pSource->UnadviseSink(_activeLanguageProfileNotifySinkCookie);
	}

	_activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
}
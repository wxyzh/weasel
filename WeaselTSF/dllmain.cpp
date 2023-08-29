// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Globals.h"
#include "test.h"
#ifdef TEST
#ifdef _M_X64
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // _M_X64
#endif // TEST

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hInstance;
#ifdef TEST
#ifdef _M_X64
		google::InitGoogleLogging("TextServiceFramework.log");
#endif // DEBUG
#endif // TEST
		if (!InitializeCriticalSectionAndSpinCount(&g_cs, 0))
			return FALSE;
		break;

	case DLL_PROCESS_DETACH:
#ifdef TEST
#ifdef _M_X64
		google::ShutdownGoogleLogging();
#endif // DEBUG
#endif // TEST
		DeleteCriticalSection(&g_cs);
		break;
	}
	return TRUE;
}


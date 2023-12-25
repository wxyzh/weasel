// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Globals.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#pragma comment(lib, "glog.lib")
#endif // TEST


BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hInstance;
#ifdef TEST
		google::InitGoogleLogging("TextServiceFramework.log");
#endif // TEST
		if (!InitializeCriticalSectionAndSpinCount(&g_cs, 0))
			return FALSE;
		break;

	case DLL_PROCESS_DETACH:
#ifdef TEST
		google::ShutdownGoogleLogging();
#endif // TEST
		DeleteCriticalSection(&g_cs);
		break;
	}
	return TRUE;
}


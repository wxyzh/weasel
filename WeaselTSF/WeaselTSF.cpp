module;
#include "stdafx.h"
#include <WeaselCommon.h>
#include "Globals.h"
#include <resource.h>
#include <psapi.h>
#include "test.h"
#ifdef TEST
#ifdef _M_X64
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#include <filesystem>
#pragma comment(lib, "psapi.lib")
namespace fs = std::filesystem;
#endif // _M_X64
#endif // TEST

//#pragma data_seg("Shared")
//bool g_checked = true;
//#pragma data_seg()
//
//#pragma comment(linker, "/SECTION:Shared,RWS")
module WeaselTSF;
import CandidateList;
import LanguageBar;
import Compartment;
import ResponseParser;
import WeaselUtility;
// import GenerateDump;

static void error_message(const WCHAR *msg)
{
	static DWORD next_tick = 0;
	DWORD now = GetTickCount();
	if (now > next_tick)
	{
		next_tick = now + 10000;  // (ms)
		MessageBox(NULL, msg, TEXTSERVICE_DESC, MB_ICONERROR | MB_OK);
	}
}

WeaselTSF::WeaselTSF()
{
	_cRef = 1;

	_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;

	_dwTextEditSinkCookie = TF_INVALID_COOKIE;
	_dwTextLayoutSinkCookie = TF_INVALID_COOKIE;
	_activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;

	_cand.Attach(new CCandidateList(*this));
	SetBit(8);		// _bitset[8]: _SupportDisplayAttribute
	SetBit(17);		// _bitset[17]: _CaretFollowing
	
	DllAddRef();
	// CatchUnhandledException();

#ifdef TEST
#ifdef _M_X64
	auto pid = GetCurrentProcessId();

	auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

	std::wstring name;
	name.reserve(MAX_PATH);

	GetProcessImageFileName(hProcess, name.data(), name.capacity());
	CloseHandle(hProcess);
	name = name.data();
	LOG(INFO) << std::format("Process {} starting log. AppName: {}", pid, fs::path(name).filename().string()).data();
#endif // _M_X64
#endif // TEST
}

WeaselTSF::~WeaselTSF()
{
	DllRelease();
}

STDAPI WeaselTSF::QueryInterface(REFIID riid, void **ppvObject)
{
	if (ppvObject == NULL)
		return E_INVALIDARG;

	*ppvObject = NULL;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor))
		*ppvObject = (ITfTextInputProcessor*)this;
	else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx))
		*ppvObject = (ITfTextInputProcessorEx*)this;
	else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
		*ppvObject = (ITfThreadMgrEventSink*)this;
	else if (IsEqualIID(riid, IID_ITfTextEditSink))
		*ppvObject = (ITfTextEditSink*)this;
	else if (IsEqualIID(riid, IID_ITfTextLayoutSink))
		*ppvObject = (ITfTextLayoutSink*)this;
	else if (IsEqualIID(riid, IID_ITfKeyEventSink))
		*ppvObject = (ITfKeyEventSink*)this;
	else if (IsEqualIID(riid, IID_ITfActiveLanguageProfileNotifySink))
		*ppvObject = (ITfActiveLanguageProfileNotifySink*)this;
	else if (IsEqualIID(riid, IID_ITfCompositionSink))
		*ppvObject = (ITfCompositionSink*)this;
	else if (IsEqualIID(riid, IID_ITfEditSession))
		*ppvObject = (ITfEditSession*)this;
	else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
		*ppvObject = (ITfThreadFocusSink*)this;
	else if (IsEqualIID(riid, IID_ITfDisplayAttributeProvider))
		*ppvObject = (ITfDisplayAttributeProvider*)this;

	if (*ppvObject)
	{
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDAPI_(ULONG) WeaselTSF::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) WeaselTSF::Release()
{
	LONG cr = --_cRef;

	assert(_cRef >= 0);

	if (_cRef == 0)
		delete this;

	return cr;
}

STDAPI WeaselTSF::Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
	return ActivateEx(pThreadMgr, tfClientId, 0U);
}

STDAPI WeaselTSF::Deactivate()
{
	m_client.EndSession();

	_InitTextEditSink();

	_UninitActiveLanguageProfileNotifySink();

	_UninitThreadMgrEventSink();

	_UninitKeyEventSink();
	_UninitPreservedKey();

	_UninitLanguageBar();

	_UninitCompartment();

	_tfClientId = TF_CLIENTID_NULL;

	_cand->Destroy();

	return S_OK;
}

STDAPI WeaselTSF::ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, DWORD dwFlags)
{
	com_ptr<ITfDocumentMgr> pDocMgrFocus;
	_activateFlags = dwFlags;
	
	_pThreadMgr = pThreadMgr;
	_tfClientId = tfClientId;	

#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::ActivateEx. _InitThreadMgrEventSink.");
#endif // _M_X64
#endif // TEST
	if (!_InitThreadMgrEventSink())
		goto ExitError;

	if ((_pThreadMgr->GetFocus(&pDocMgrFocus) == S_OK) && (pDocMgrFocus != NULL))
	{
		_InitTextEditSink(pDocMgrFocus);
	}

	if (!_InitKeyEventSink())
		goto ExitError;

#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::ActivateEx. _InitActiveLanguageProfileNotifySink");
#endif // _M_X64
#endif // TEST

	if (!_InitActiveLanguageProfileNotifySink())
	{
		goto ExitError;
	}

	if (!_InitDisplayAttributeGuidAtom())
	{
		ReSetBit(8);	// _bitset[8]: _SupportDisplayAttribute
	}

	if (!_InitPreservedKey())
		goto ExitError;	

	if (!_InitLanguageBar())
		goto ExitError;

	if (!_IsKeyboardOpen())
		_SetKeyboardOpen(TRUE);

	_InitGlobalCompartment();
	_pCompartmentConversion = std::make_unique<CCompartment>(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);

	_EnsureServerConnected();

#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::ActivateEx. _InitCompartment");
#endif // _M_X64
#endif // TEST

	if (!_InitCompartment())
		goto ExitError;	

	return S_OK;

ExitError:
	Deactivate();
	return E_FAIL;
}

STDMETHODIMP WeaselTSF::OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL isActivated)
{
#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::OnActivated. isActivated = {}", isActivated);
#endif // _M_X64
#endif // TEST
	if (!IsEqualCLSID(clsid, c_clsidTextService))
	{
		return S_OK;
	}

	if (isActivated) {
		_ShowLanguageBar(TRUE);
	}
	else {
		_DeleteCandidateList();
		_ShowLanguageBar(FALSE);
	}
	return S_OK;
}

void WeaselTSF::_EnsureServerConnected()
{
	if (!m_client.Echo())
	{
		m_client.Disconnect();
		m_client.Connect(NULL);
		m_client.StartSession();
		weasel::ResponseParser parser(NULL, NULL, &_status, NULL, &_cand->style());
		bool ok = m_client.GetResponseData(std::ref(parser));
		if (ok) {
			_UpdateLanguageBar(_status);			
		}
		if (!m_client.Echo())
		{
			VARIANT var{};
			if (SUCCEEDED(_GetGlobalCompartmentDaemon()->GetValue(&var)))
			{
				if (var.vt == VT_I4)
				{
					SetBit(0, var.bVal);	// _bitset[0]: _daemon_enable
				}
			}
			if (GetBit(0))					// _bitset[0]: _daemon_enable
			{
				execute(std::format(LR"({}\WeaselServer.exe)", WeaselRootPath()));
			}
		}
	}
}
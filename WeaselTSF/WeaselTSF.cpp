module;
#include "stdafx.h"
#include <WeaselCommon.h>
#include "Globals.h"
#include <resource.h>
#include <filesystem>
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST

namespace fs = std::filesystem;

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
	SetBit(WeaselFlag::SUPPORT_DISPLAY_ATTRIBUTE);
	SetBit(WeaselFlag::CARET_FOLLOWING);

	DllAddRef();
	// CatchUnhandledException();
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
	else if (IsEqualIID(riid, IID_ITfDisplayAttributeProvider))
		*ppvObject = (ITfDisplayAttributeProvider*)this;
	else if (IsEqualIID(riid, IID_ITfCleanupContextDurationSink))
		*ppvObject = (ITfCleanupContextDurationSink*)this;

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
	_UninitCleanupContextDurationSink();

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

	_InitWeaselData();

#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::ActivateEx. _InitThreadMgrEventSink.");
#endif // TEST
	if (!_InitThreadMgrEventSink())
		goto ExitError;

	if ((_pThreadMgr->GetFocus(&pDocMgrFocus) == S_OK) && (pDocMgrFocus != NULL))
	{
		_InitTextEditSink(pDocMgrFocus);
	}

	if (!_InitCleanupContextDurationSink())
		goto ExitError;

	if (!_InitKeyEventSink())
		goto ExitError;

#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::ActivateEx. _InitActiveLanguageProfileNotifySink");
#endif // TEST

	if (!_InitActiveLanguageProfileNotifySink())
	{
		goto ExitError;
	}

	if (!_InitDisplayAttributeGuidAtom())
	{
		ResetBit(WeaselFlag::SUPPORT_DISPLAY_ATTRIBUTE);
	}

	if (!_InitLanguageBar())
		goto ExitError;

	if (!_IsKeyboardOpen())
		_SetKeyboardOpen(TRUE);

	_InitGlobalCompartment();
	_pCompartmentConversion = std::make_unique<CCompartment>(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);

	_EnsureServerConnected();

#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::ActivateEx. _InitCompartment. supportDiaplayAttribute = {:s}", GetBit(WeaselFlag::SUPPORT_DISPLAY_ATTRIBUTE));
#endif // TEST

	if (!_InitCompartment())
		goto ExitError;	

	if (GetBit(WeaselFlag::PRESERVED_KEY_SWITCH))
		_InitPreservedKey();

	return S_OK;

ExitError:
	Deactivate();
	return E_FAIL;
}

STDMETHODIMP WeaselTSF::OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL isActivated)
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnActivated. isActivated = {}", isActivated);
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
			_UpdateLanguageBar();			
		}
		if (!m_client.Echo())
		{
			UpdateGlobalCompartment();
			if (GetBit(WeaselFlag::DAEMON_ENABLE))
			{
				execute(std::format(LR"({}\WeaselServer.exe)", WeaselRootPath()));
			}
		}
	}
}

void WeaselTSF::_InitWeaselData()
{
	auto pid = GetCurrentProcessId();
	auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

	std::wstring name;
	DWORD len{ 1024 };
	name.reserve(len);

	auto ret = QueryFullProcessImageName(hProcess, 0, name.data(), &len);
	CloseHandle(hProcess);
	name = name.data();

	if (fs::path(name).filename().wstring() == L"acad.exe")
	{
		SetBit(WeaselFlag::AUTOCAD);
	}

	if (fs::path(name).filename().wstring() == L"wezterm-gui.exe")
	{
		SetBit(WeaselFlag::WEZTERM_FIRST_KEY);
	}

	if (fs::path(name).filename().wstring() == L"firefox.exe")
	{
		SetBit(WeaselFlag::FIREFOX);
	}

	if (fs::path(name).filename().wstring() == L"dota2.exe")
	{
		SetBit(WeaselFlag::GAME_MODE_SELF_REDRAW);
		ResetBit(WeaselFlag::CARET_FOLLOWING);
		_cand->SetCaretFollowing(GetBit(WeaselFlag::CARET_FOLLOWING));
	}

	for (size_t i{}; i < _gameNames.size(); ++i)
	{
		if (fs::path(name).filename().wstring() == _gameNames[i])
		{
			SetBit(WeaselFlag::GAME_MODE);
			if (i == 0)
			{
				SetBit(WeaselFlag::GAME_WAR3);
			}
		}
	}
#ifdef TEST
	LOG(INFO) << std::format("Process {} starting log. AppName: {}, length = {}, is successful? {:s}", pid, fs::path(name).filename().string(), name.size(), static_cast<bool>(ret)).data();
#endif // TEST
}
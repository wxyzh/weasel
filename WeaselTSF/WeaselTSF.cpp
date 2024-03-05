module;
#include "stdafx.h"
#include <WeaselCommon.h>
#include "Globals.h"
#include <resource.h>
#include <filesystem>
#include "test.h"

namespace fs = std::filesystem;

module WeaselTSF;
import CandidateList;
import LanguageBar;
import Compartment;
import ResponseParser;
import WeaselUtility;
// import GenerateDump;

static void error_message(const WCHAR* msg)
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
	_dwThreadFocusSinkCookie = TF_INVALID_COOKIE;

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

STDAPI WeaselTSF::QueryInterface(REFIID riid, void** ppvObject)
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
	else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
		*ppvObject = (ITfThreadFocusSink*)this;

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

STDAPI WeaselTSF::Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId)
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

	if (!GetBit(WeaselFlag::INIT_LANGUAGEBAR_FAILED))
		_UninitLanguageBar();

	_UninitCompartment();
	_UninitThreadFocusSink();

	_tfClientId = TF_CLIENTID_NULL;

	_cand->Destroy();

#ifdef TEST
	if (GetBit(WeaselFlag::WEASEL_TSF_DEBUG) && m_hConsole)
	{
		::FreeConsole();
	}
#endif // TEST	

	return S_OK;
}

STDAPI WeaselTSF::ActivateEx(ITfThreadMgr* pThreadMgr, TfClientId tfClientId, DWORD dwFlags)
{
#ifdef TEST
	std::wstring buffer{ std::format(L"From WeaselActivateEx().\n") };
	WriteConsole(buffer);
#endif // TEST

	com_ptr<ITfDocumentMgr> pDocMgrFocus;
	_activateFlags = dwFlags;

	_pThreadMgr = pThreadMgr;
	_tfClientId = tfClientId;

	_InitWeaselData();

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). InitThreadMgrEventSink\n");
	WriteConsole(buffer);
#endif // TEST
	if (!_InitThreadMgrEventSink())
		goto ExitError;

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). InitTextEditSink\n");
	WriteConsole(buffer);
#endif // TEST
	if ((_pThreadMgr->GetFocus(&pDocMgrFocus) == S_OK) && (pDocMgrFocus != NULL))
	{
		_InitTextEditSink(pDocMgrFocus);
	}

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). InitCleanupContextDurationSink\n");
	WriteConsole(buffer);
#endif // TEST
	if (!_InitCleanupContextDurationSink())
		goto ExitError;

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). InitKeyEventSink\n");
	WriteConsole(buffer);
#endif // TEST
	if (!_InitKeyEventSink())
		goto ExitError;

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). InitActiveLanguageProfileNotifySink\n");
	WriteConsole(buffer);
#endif // TEST
	if (!_InitActiveLanguageProfileNotifySink())
	{
		goto ExitError;
	}

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). InitDisplayAttributeGuidAtom\n");
	WriteConsole(buffer);
#endif // TEST
	if (!_InitDisplayAttributeGuidAtom())
	{
		ResetBit(WeaselFlag::SUPPORT_DISPLAY_ATTRIBUTE);
	}

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). InitLanguageBar\n");
	WriteConsole(buffer);
#endif // TEST
	if (!_InitLanguageBar())
		SetBit(WeaselFlag::INIT_LANGUAGEBAR_FAILED);

	if (!_InitThreadFocusSink())
		goto ExitError;

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). IsKeyboardOpen\n");
	WriteConsole(buffer);
#endif // TEST
	if (!_IsKeyboardOpen())
		_SetKeyboardOpen(TRUE);

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). InitGlobalCompartment\n");
	WriteConsole(buffer);
#endif // TEST
	if (!GetBit(WeaselFlag::PLANTS_VS_ZOMBIES))
		_InitGlobalCompartment();
	_pCompartmentConversion = std::make_unique<CCompartment>(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);

	_EnsureServerConnected();

	if (!_InitCompartment())
		goto ExitError;

	if (GetBit(WeaselFlag::PRESERVED_KEY_SWITCH))
		_InitPreservedKey();

#ifdef TEST
	buffer = std::format(L"From WeaselActivateEx(). End\n");
	WriteConsole(buffer);
#endif // TEST

	return S_OK;

ExitError:
	Deactivate();
	return E_FAIL;
}

STDMETHODIMP WeaselTSF::OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL isActivated)
{
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
				Sleep(150);
				if (!m_client.Echo())
				{
					m_client.Disconnect();
					m_client.Connect(NULL);
					m_client.StartSession();
				}
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
	name = fs::path(name).filename().wstring();

	if (name == L"acad.exe")
	{
		SetBit(WeaselFlag::AUTOCAD);
	}

	if (name == L"wezterm-gui.exe")
	{
		SetBit(WeaselFlag::WEZTERM_FIRST_KEY);
	}

	if (name == L"firefox.exe")
	{
		SetBit(WeaselFlag::FIREFOX);
	}

	if (name == L"dota2.exe")
	{
		SetBit(WeaselFlag::GAME_MODE_SELF_REDRAW);
		ResetBit(WeaselFlag::CARET_FOLLOWING);
		_cand->SetCaretFollowing(GetBit(WeaselFlag::CARET_FOLLOWING));
		ReadConfiguration(ConfigFlag::FALLBACK_POSITION);
	}

	if (name == L"PlantsVsZombies.exe")
	{
		SetBit(WeaselFlag::PLANTS_VS_ZOMBIES);
	}

#ifdef TEST
	if (name != L"conhost.exe" && name != L"explorer.exe")
	{
		AllocConsole();
		SetBit(WeaselFlag::WEASEL_TSF_DEBUG);
		m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		std::wstring buffer{ std::format(L"ActivateEx. Current Process: {}\n", name) };
		WriteConsole(buffer);
	}
#endif // TEST	

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
}

void WeaselTSF::WriteConsole(std::wstring_view buffer)
{
	if (GetBit(WeaselFlag::WEASEL_TSF_DEBUG) && m_hConsole)
	{
		::WriteConsole(m_hConsole, buffer.data(), buffer.size(), nullptr, 0);
	}
}
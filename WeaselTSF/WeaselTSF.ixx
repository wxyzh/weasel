module;
#include "stdafx.h"
#include "Globals.h"
#include <WeaselCommon.h>
export module WeaselTSF;
import WeaselIPC;
import <bitset>;
import <memory>;
import WeaselFlag;

class CCandidateList;
class CLangBarItemButton;
class CCompartment;
class CCompartmentEventSink;

export
{
	class WeaselTSF :
		public ITfTextInputProcessorEx,
		public ITfThreadMgrEventSink,
		public ITfTextEditSink,
		public ITfTextLayoutSink,
		public ITfKeyEventSink,
		public ITfCompositionSink,
		public ITfActiveLanguageProfileNotifySink,
		public ITfEditSession,
		public ITfDisplayAttributeProvider,
		public ITfCleanupContextDurationSink,
		public ITfThreadFocusSink
	{
	public:
		WeaselTSF();
		~WeaselTSF();

		/* IUnknown */
		STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();

		/* ITfTextInputProcessor */
		STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId);
		STDMETHODIMP Deactivate();

		/* ITfTextInputProcessorEx */
		STDMETHODIMP ActivateEx(ITfThreadMgr* pThreadMgr, TfClientId tfClientId, DWORD dwFlags);

		/* ITfThreadMgrEventSink */
		STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr* pDocMgr);
		STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr* pDocMgr);
		STDMETHODIMP OnSetFocus(ITfDocumentMgr* pDocMgrFocus, ITfDocumentMgr* pDocMgrPrevFocus);
		STDMETHODIMP OnPushContext(ITfContext* pContext);
		STDMETHODIMP OnPopContext(ITfContext* pContext);

		/* ITfTextEditSink */
		STDMETHODIMP OnEndEdit(ITfContext* pic, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord);

		/* ITfTextLayoutSink */
		STDMETHODIMP OnLayoutChange(ITfContext* pContext, TfLayoutCode lcode, ITfContextView* pContextView);

		/* ITfKeyEventSink */
		STDMETHODIMP OnSetFocus(BOOL fForeground);
		STDMETHODIMP OnTestKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
		STDMETHODIMP OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
		STDMETHODIMP OnTestKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
		STDMETHODIMP OnKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
		STDMETHODIMP OnPreservedKey(ITfContext* pContext, REFGUID rguid, BOOL* pfEaten);

		/* ITfCompositionSink */
		STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition* pComposition);

		/* ITfEditSession */
		STDMETHODIMP DoEditSession(TfEditCookie ec);

		/* ITfActiveLanguageProfileNotifySink */
		STDMETHODIMP OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL isActivated);

		// ITfDisplayAttributeProvider
		STDMETHODIMP EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum);
		STDMETHODIMP GetDisplayAttributeInfo(__RPC__in REFGUID guidInfo, __RPC__deref_out_opt ITfDisplayAttributeInfo** ppInfo);

		/* ITfThreadFocusSink */
		STDMETHODIMP OnSetThreadFocus();
		STDMETHODIMP OnKillThreadFocus();

		/* ITfCompartmentEventSink */
		// STDMETHODIMP OnChange(_In_ REFGUID guid);

		/* Compartments */
		BOOL _IsKeyboardDisabled();
		BOOL _IsKeyboardOpen();
		HRESULT _SetKeyboardOpen(BOOL fOpen);

		/* ITfCleanupContextDurationSink */		
		STDMETHODIMP OnStartCleanupContext();
		STDMETHODIMP OnEndCleanupContext();

		/* Composition */
		void _StartComposition(ITfContext* pContext, bool not_inline_preedit);
		void _EndComposition(ITfContext* pContext, BOOL clear);
		BOOL _ShowInlinePreedit(ITfContext* pContext, const std::shared_ptr<weasel::Context> context);
		void _UpdateComposition(ITfContext* pContext);
		BOOL _IsComposing();
		void _SetComposition(ITfComposition* pComposition);
		void _SetCompositionPosition(const RECT& rc);
		BOOL _UpdateCompositionWindow(ITfContext* pContext);
		void _FinalizeComposition();
		void _AbortComposition(bool clear = true);

		/* Language bar */
		HWND _GetFocusedContextWindow();
		void _HandleLangBarMenuSelect(UINT wID);

		/* IPC */
		void _EnsureServerConnected();

		/* UI */
		void _UpdateUI(const weasel::Context& ctx, const weasel::Status& status);
		void _StartUI();
		void _EndUI();
		void _ShowUI();
		void _HideUI();
		com_ptr<ITfContext> _GetUIContextDocument();

		/* Display Attribute */
		void _ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext* pContext);
		BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext* pContext, ITfRange* pRangeComposition);
		BOOL _InitDisplayAttributeGuidAtom();

		com_ptr<ITfThreadMgr> _GetThreadMgr() { return _pThreadMgr; }

		void HandleUICallback(int* const sel, int* const hov, bool* const next, bool* const scroll_next);

		HRESULT _GetCompartmentDWORD(_Out_ DWORD& value, const GUID guid);
		HRESULT _SetCompartmentDWORD(DWORD value, const GUID guid);
		HRESULT _InitGlobalCompartment();

		void SetBit(WeaselFlag flag) { _bitset.set(static_cast<int>(flag)); }
		void SetBit(WeaselFlag flag, bool value) { _bitset.set(static_cast<int>(flag), value); }
		void ResetBit(WeaselFlag flag) { _bitset.reset(static_cast<int>(flag)); }
		bool GetBit(WeaselFlag flag) const { return _bitset[static_cast<int>(flag)]; }
		void Flip(WeaselFlag flag) { _bitset.flip(static_cast<size_t>(flag)); }

		bool execute(std::wstring_view cmd, std::wstring_view args = L"");
		bool explore(std::wstring_view path);
		void SetRect(const RECT& rc) { m_rcFallback = rc; }
		RECT GetRect() const { return m_rcFallback; }
		WCHAR GetInput() const { return static_cast<WCHAR>(_keycode); }

		bool RetryKey();
		void RetryFailedEvent();
		SIZE GetScreenResolution();
		void SimulatingKeyboardEvents(unsigned short code);

		// write in if true, read out if false
		void UpdateGlobalCompartment(bool in = true);
		void SetHWND(HWND hwnd) { m_hwnd = hwnd; }

		bool ReadConfiguration();
		bool WriteConfiguration();
		bool StoreTextServiceHandle(HKL hkl);

		TfClientId GetClientId() const { return _tfClientId; }

		void WriteConsole(std::wstring_view buffer);

	private:
		// ui callback functions
		void _SelectCandidateOnCurrentPage(const int index);
		void _HandleMouseHoverEvent(const int index);
		void _HandleMousePageEvent(bool* const nextPage, bool* const scrollNextPage);
		/* TSF Related */
		BOOL _InitThreadMgrEventSink();
		void _UninitThreadMgrEventSink();

		BOOL _InitTextEditSink(ITfDocumentMgr* pDocMgr = nullptr);

		BOOL _InitKeyEventSink();
		void _UninitKeyEventSink();
		void _ProcessKeyEvent(WPARAM wParam, LPARAM lParam, BOOL* pfEaten, bool isTest = false);

		BOOL _InitActiveLanguageProfileNotifySink();
		void _UninitActiveLanguageProfileNotifySink();

		BOOL _InitPreservedKey();
		void _UninitPreservedKey();

		BOOL _InitLanguageBar();
		void _UninitLanguageBar();
		bool _UpdateLanguageBar();
		void _ShowLanguageBar(BOOL show);
		void _EnableLanguageBar(BOOL enable);

		BOOL _InitThreadFocusSink();
		void _UninitThreadFocusSink();

		BOOL _InsertText(ITfContext* pContext, std::wstring_view text);

		void _DeleteCandidateList();

		BOOL _InitCompartment();
		void _UninitCompartment();
		HRESULT _HandleCompartment(REFGUID guidCompartment);

		BOOL _InitCleanupContextDurationSink();
		void _UninitCleanupContextDurationSink();

		void _InitWeaselData();

		bool isImmersive() const {
			return (_activateFlags & TF_TMF_IMMERSIVEMODE) != 0;
		}

	private:
		com_ptr<ITfThreadMgr> _pThreadMgr;
		TfClientId _tfClientId;
		DWORD _dwThreadMgrEventSinkCookie;

		com_ptr<ITfContext> _pTextEditSinkContext;
		DWORD _dwTextEditSinkCookie, _dwTextLayoutSinkCookie;
		BYTE _lpbKeyState[256];

		com_ptr<ITfContext> _pEditSessionContext;
		std::wstring _editSessionText;

		std::unique_ptr<CCompartment> _pCompartmentConversion;
		com_ptr<ITfCompartment> _pGlobalCompartment;
		com_ptr<CCompartmentEventSink> _pKeyboardCompartmentSink;
		com_ptr<CCompartmentEventSink> _pConversionCompartmentSink;

		com_ptr<ITfComposition> _pComposition;

		com_ptr<CLangBarItemButton> _pLangBarButton;

		com_ptr<CCandidateList> _cand;

		com_ptr<ITfDocumentMgr> _pDocMgrLastFocused;

		LONG _cRef;	// COM ref count

		/* CUAS Candidate Window Position Workaround */
		bool _fCUASWorkaroundTested{};
		bool _fCUASWorkaroundEnabled{};

		/* Test Key Down/Up pending. */
		bool _fTestKeyDownPending{};
		bool _fTestKeyUpPending{};

		/* Weasel Related */
		weasel::Client m_client;
		DWORD _activateFlags;

		/* IME status */
		weasel::Status _status;

		// guidatom for the display attibute.
		TfGuidAtom _gaDisplayAttributeInput;

		// The cookie of ActiveLanguageProfileNotifySink
		DWORD _activeLanguageProfileNotifySinkCookie;

		DWORD _dwThreadFocusSinkCookie;

		RECT m_rcFallback{};
		HWND m_hwnd{};
		std::bitset<32> _bitset{};

		std::wstring _schema_id{};
		unsigned _keycode{};
		unsigned short _lastKey{};
		bool m_hasPreedit{};
		int m_preeditCount{};

		DWORD m_globalCompartment{ 0xFC00'0003 };
		
		TF_PRESERVEDKEY _preservedKeyGameMode;			// Ctrl+Shift+G
		TF_PRESERVEDKEY _preservedKeyCaretFollowing;	// Ctrl+Shift+F
		TF_PRESERVEDKEY _preservedKeyDaemon;			// Ctrl+Shift+D
		// TF_PRESERVEDKEY _preservedKeyImeMode;			// Ctrl+9
		std::array<std::wstring, 2> _gameNames
		{
			L"War3.exe",
			L"WoW.exe"
		};

		HANDLE m_hConsole{};
	};
}
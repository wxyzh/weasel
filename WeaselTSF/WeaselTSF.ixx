module;
#include "Globals.h"
export module WeaselTSF;
import WeaselCommon;
import WeaselIPC;
import <bitset>;
import <memory>;

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
		public ITfThreadFocusSink,
		public ITfActiveLanguageProfileNotifySink,
		public ITfEditSession,
		public ITfDisplayAttributeProvider,
		public ITfStatusSink
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

		// ITfThreadFocusSink
		STDMETHODIMP OnSetThreadFocus();
		STDMETHODIMP OnKillThreadFocus();

		/* ITfCompositionSink */
		STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition* pComposition);

		/* ITfEditSession */
		STDMETHODIMP DoEditSession(TfEditCookie ec);

		/* ITfActiveLanguageProfileNotifySink */
		STDMETHODIMP OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL isActivated);

		// ITfDisplayAttributeProvider
		STDMETHODIMP EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum);
		STDMETHODIMP GetDisplayAttributeInfo(__RPC__in REFGUID guidInfo, __RPC__deref_out_opt ITfDisplayAttributeInfo** ppInfo);

		// ITfStatusSink
		STDMETHODIMP OnStatusChange(__in ITfContext* pContext, __in DWORD dwFlags);

		/* ITfCompartmentEventSink */
		// STDMETHODIMP OnChange(_In_ REFGUID guid);

		/* Compartments */
		BOOL _IsKeyboardDisabled();
		BOOL _IsKeyboardOpen();
		HRESULT _SetKeyboardOpen(BOOL fOpen);

		/* Composition */
		void _StartComposition(ITfContext* pContext, BOOL fCUASWorkaroundEnabled);
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

		void InsertText(std::wstring_view str, const size_t index);

		HRESULT _GetCompartmentDWORD(_Out_ DWORD& value, const GUID guid);
		HRESULT _SetCompartmentDWORD(DWORD value, const GUID guid);
		HRESULT _InitGlobalCompartment();

		void SetBit(int index) { _bitset.set(index); }
		void SetBit(int index, bool value) { _bitset.set(index, value); }
		void ReSetBit(int index) { _bitset.reset(index); }
		bool GetBit(int index) const { return _bitset[index]; }

		bool execute(std::wstring_view cmd, std::wstring_view args);
		bool explore(std::wstring_view path);
		com_ptr<ITfCompartment> _GetGlobalCompartmentDaemon() { return _pGlobalCompartmentDaemon; }

	private:
		/* TSF Related */
		BOOL _InitThreadMgrEventSink();
		void _UninitThreadMgrEventSink();

		BOOL _InitTextEditSink(ITfDocumentMgr* pDocMgr = nullptr);

		BOOL _InitKeyEventSink();
		void _UninitKeyEventSink();
		void _ProcessKeyEvent(WPARAM wParam, LPARAM lParam, BOOL* pfEaten);

		BOOL _InitActiveLanguageProfileNotifySink();
		void _UninitActiveLanguageProfileNotifySink();

		BOOL _InitPreservedKey();
		void _UninitPreservedKey();

		BOOL _InitLanguageBar();
		void _UninitLanguageBar();
		void _UpdateLanguageBar(weasel::Status& stat);
		void _ShowLanguageBar(BOOL show);
		void _EnableLanguageBar(BOOL enable);

		BOOL _InsertText(ITfContext* pContext, std::wstring_view text);

		void _DeleteCandidateList();

		BOOL _InitCompartment();
		void _UninitCompartment();
		HRESULT _HandleCompartment(REFGUID guidCompartment);

		BOOL _InitThreadFocusSink();
		void _UninitThreadFocusSink();

		BOOL _InitStatusSink();
		void _UninitStatusSink();

		bool isImmersive() const {
			return (_activateFlags & TF_TMF_IMMERSIVEMODE) != 0;
		}

		com_ptr<ITfThreadMgr> _pThreadMgr;
		TfClientId _tfClientId;
		DWORD _dwThreadMgrEventSinkCookie;
		DWORD _dwThreadFocusSinkCookie;

		com_ptr<ITfContext> _pTextEditSinkContext;
		DWORD _dwTextEditSinkCookie, _dwTextLayoutSinkCookie;
		BYTE _lpbKeyState[256];

		com_ptr<ITfContext> _pEditSessionContext;
		std::wstring _editSessionText;

		std::unique_ptr<CCompartment> _pCompartmentConversion;
		com_ptr<ITfCompartment> _pGlobalCompartmentDaemon;
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

		// The cookie of StatusSink
		DWORD _dwStatusSinkCookie;

		// _bitset[0]:  _daemon_enable
		// _bitset[1]:  _ascii_mode
		// _bitset[2]:  _full_shape
		// _bitset[3]:  _ascii_punct
		// _bitset[4]:  _InitInputMethodState
		// _bitset[5]:  _InlinePreedit
		// _bitset[6]:  _BeginComposition
		// _bitset[7]:  _FocusChanged
		// _bitset[8]:  _SupportDisplayAttribute
		// _bitset[9]:
		// _bitset[10]: 
		// _bitset[11]:
		// _bitset[12]: 
		// _bitset[13]: 
		// _bitset[14]: 
		// _bitset[15]:  
		std::bitset<16> _bitset{};
	};
}
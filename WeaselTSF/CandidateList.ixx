module;
#include "stdafx.h"
#include <WeaselUI.h>
#include <WeaselCommon.h>
#include "ctffunc.h"
export module CandidateList;
import WeaselTSF;

export
{
	class CCandidateList :
		public ITfIntegratableCandidateListUIElement,
		public ITfCandidateListUIElementBehavior
	{
	public:
		// com_ptr智能指针作为参数传送会触发COM引用计数，测试时日志里会因为这个COM引用计数不会递减至0
		// 故将其类型改为了引用
		CCandidateList(WeaselTSF& textService);
		~CCandidateList();

		// IUnknown
		STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		// ITfUIElement
		STDMETHODIMP GetDescription(BSTR* pbstr);
		STDMETHODIMP GetGUID(GUID* pguid);
		STDMETHODIMP Show(BOOL showCandidateWindow);
		STDMETHODIMP IsShown(BOOL* pIsShow);

		// ITfCandidateListUIElement
		STDMETHODIMP GetUpdatedFlags(DWORD* pdwFlags);
		STDMETHODIMP GetDocumentMgr(ITfDocumentMgr** ppdim);
		STDMETHODIMP GetCount(UINT* pCandidateCount);
		STDMETHODIMP GetSelection(UINT* pSelectedCandidateIndex);
		STDMETHODIMP GetString(UINT uIndex, BSTR* pbstr);
		STDMETHODIMP GetPageIndex(UINT* pIndex, UINT uSize, UINT* puPageCnt);
		STDMETHODIMP SetPageIndex(UINT* pIndex, UINT uPageCnt);
		STDMETHODIMP GetCurrentPage(UINT* puPage);

		// ITfCandidateListUIElementBehavior methods
		STDMETHODIMP SetSelection(UINT nIndex);
		STDMETHODIMP Finalize(void);
		STDMETHODIMP Abort(void);

		// ITfIntegratableCandidateListUIElement methods
		STDMETHODIMP SetIntegrationStyle(GUID guidIntegrationStyle);
		STDMETHODIMP GetSelectionStyle(_Out_ TfIntegratableCandidateListSelectionStyle* ptfSelectionStyle);
		STDMETHODIMP OnKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pIsEaten);
		STDMETHODIMP ShowCandidateNumbers(_Out_ BOOL* pIsShow);
		STDMETHODIMP FinalizeExactCompositionString();

		/* Update */
		void UpdateUI(const weasel::Context& ctx, const weasel::Status& status);
		void UpdateStyle(const weasel::UIStyle& sty);
		void UpdateInputPosition(RECT const& rc);
		void Destroy();
		void StartUI();
		void EndUI();
		void SetCaretFollowing(bool following);

		void SetThreadFocus();
		void KillThreadFocus();

		com_ptr<ITfContext> GetContextDocument();
		bool GetIsReposition();

		weasel::UIStyle& style();

	private:
		//void _UpdateOwner();
		HWND _GetActiveWnd();
		HRESULT _UpdateUIElement();

		// for CCandidateList::EndUI(), after ending composition || WeaselTSF::_EndUI()
		void _DisposeUIWindow();
		// for CCandidateList::Destroy(), when inputing app exit
		void _DisposeUIWindowAll();
		void _MakeUIWindow();

		std::unique_ptr<weasel::UI> _ui;
		// 改为引用后，成员变量的初始化顺序是依据定义时的顺序，需要调整下定义的位置
		WeaselTSF& _tsf;
		DWORD _cRef;
		DWORD uiid{};
		TfIntegratableCandidateListSelectionStyle _selectionStyle = STYLE_ACTIVE_SELECTION;

		BOOL _pbShow;
		weasel::UIStyle _style;

		com_ptr<ITfContext> _pContextDocument;
	};
}
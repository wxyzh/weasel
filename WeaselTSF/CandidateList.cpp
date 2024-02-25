module;
#include "stdafx.h"
#include <WeaselUI.h>
#include <WeaselCommon.h>
#include "Globals.h"
module CandidateList;
import WeaselUtility;
using namespace weasel;

CCandidateList::CCandidateList(WeaselTSF& textService) :
	_tsf(textService)
{
	// _pReadingInformation.Attach(new CReadingInformation(_tsf, &_ui));
	_cRef = 1;
	_pbShow = TRUE;
	_ui.SetSelectCallback([this](int* const sel, int* const hov, bool* const next, bool* const scroll_next) 
		{ 
			_tsf.HandleUICallback(sel, hov, next, scroll_next); 
		});
	_ui.SetRectCallback([this](const RECT& rc) { _tsf.SetRect(rc); });
}

CCandidateList::~CCandidateList()
{

}

STDMETHODIMP CCandidateList::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_INVALIDARG;
	}

	*ppvObj = nullptr;

	if (IsEqualIID(riid, IID_ITfUIElement) ||
		IsEqualIID(riid, IID_ITfCandidateListUIElement) ||
		IsEqualIID(riid, IID_ITfCandidateListUIElementBehavior))
	{
		*ppvObj = (ITfCandidateListUIElementBehavior*)this;
	}
	else if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(ITfIntegratableCandidateListUIElement)))
	{
		*ppvObj = (ITfIntegratableCandidateListUIElement*)this;
	}

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CCandidateList::AddRef(void)
{
	return ++_cRef;
}

STDMETHODIMP_(ULONG) CCandidateList::Release(void)
{
	LONG cr = --_cRef;

	assert(_cRef >= 0);

	if (_cRef == 0)
	{
		delete this;
	}

	return cr;
}

STDMETHODIMP CCandidateList::GetDescription(BSTR* pbstr)
{
	static auto str = SysAllocString(L"Candidate List");
	if (pbstr)
	{
		*pbstr = str;
	}
	return S_OK;
}

STDMETHODIMP CCandidateList::GetGUID(GUID* pguid)
{
	/// 36c3c795-7159-45aa-ab12-30229a51dbd3
	*pguid = { 0x36c3c795, 0x7159, 0x45aa, { 0xab, 0x12, 0x30, 0x22, 0x9a, 0x51, 0xdb, 0xd3 } };
	return S_OK;
}

STDMETHODIMP CCandidateList::Show(BOOL showCandidateWindow)
{
	com_ptr<ITfCompartmentMgr> pCompartmentMgr;
	if (_pContextDocument)
	{
		if (SUCCEEDED(_pContextDocument->QueryInterface(&pCompartmentMgr)))
		{
			// Update a hidden compartment to generate
			// IMN_OPENCANDIDATE/IMN_CLOSECANDIDATE notifications for the 
			// application compatibility.
			com_ptr<ITfCompartment> pCompartment;
			if (SUCCEEDED(pCompartmentMgr->GetCompartment(WEASEL_CUAS_CANDIDATE_MESSAGE_COMPARTMENT, &pCompartment)))
			{
				VARIANT var{};
				var.vt = VT_I4;
				var.lVal = showCandidateWindow ? TRUE : FALSE;
				pCompartment->SetValue(_tsf.GetClientId(), &var);
			}
		}
	}

	if (showCandidateWindow)
	{
		_ui.Show();
	}
	else
	{
		_ui.Hide();
	}
	return S_OK;
}

STDMETHODIMP CCandidateList::IsShown(BOOL* pbShow)
{
	*pbShow = _ui.IsShown();
	return S_OK;
}

STDMETHODIMP CCandidateList::GetUpdatedFlags(DWORD* pdwFlags)
{
	if (!pdwFlags)
		return E_INVALIDARG;

	*pdwFlags = _flags;
	_flags = 0;

	return S_OK;
}

STDMETHODIMP CCandidateList::GetDocumentMgr(ITfDocumentMgr** ppdim)
{
	*ppdim = nullptr;
	auto pThreadMgr = _tsf._GetThreadMgr();
	if (pThreadMgr == nullptr)
	{
		return E_FAIL;
	}
	if (FAILED(pThreadMgr->GetFocus(ppdim)) || (*ppdim == nullptr))
	{
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP CCandidateList::GetCount(UINT* pCandidateCount)
{
	if (pCandidateCount == nullptr)
		return E_INVALIDARG;

	*pCandidateCount = _ui.ctx().cinfo.candies.size();
	return S_OK;
}

STDMETHODIMP CCandidateList::GetSelection(UINT* pSelectedCandidateIndex)
{
	if (_ui.ctx().cinfo.candies.empty())
		return S_FALSE;

	*pSelectedCandidateIndex = _ui.ctx().cinfo.highlighted;
	return S_OK;
}

STDMETHODIMP CCandidateList::GetString(UINT uIndex, BSTR* pbstr)
{
	*pbstr = nullptr;
	auto& cinfo = _ui.ctx().cinfo;
	if (uIndex >= cinfo.candies.size())
		return E_INVALIDARG;

	auto& str = cinfo.candies[uIndex].str;
	*pbstr = SysAllocStringLen(str.c_str(), str.size());

	return S_OK;
}

STDMETHODIMP CCandidateList::GetPageIndex(UINT* pIndex, UINT uSize, UINT* puPageCnt)
{
	if (!puPageCnt)
		return E_INVALIDARG;
	*puPageCnt = 1;
	if (pIndex) {
		if (uSize < *puPageCnt) {
			return E_INVALIDARG;
		}
		*pIndex = 0;
	}
	return S_OK;
}

STDMETHODIMP CCandidateList::SetPageIndex(UINT* pIndex, UINT uPageCnt)
{
	if (!pIndex)
		return E_INVALIDARG;
	return S_OK;
}

STDMETHODIMP CCandidateList::GetCurrentPage(UINT* puPage)
{
	*puPage = 0;
	return S_OK;
}

STDMETHODIMP CCandidateList::SetSelection(UINT nIndex)
{
	return S_OK;
}

STDMETHODIMP CCandidateList::Finalize(void)
{
	Destroy();
	return S_OK;
}

STDMETHODIMP CCandidateList::Abort(void)
{
	_tsf._AbortComposition(true);
	return S_OK;
}

STDMETHODIMP CCandidateList::SetIntegrationStyle(GUID guidIntegrationStyle)
{
	return S_OK;
}

STDMETHODIMP CCandidateList::GetSelectionStyle(TfIntegratableCandidateListSelectionStyle* ptfSelectionStyle)
{
	* ptfSelectionStyle = _selectionStyle;
	return S_OK;
}

STDMETHODIMP CCandidateList::OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL* pIsEaten)
{
	*pIsEaten = TRUE;
	return S_OK;
}

STDMETHODIMP CCandidateList::ShowCandidateNumbers(BOOL* pIsShow)
{
	* pIsShow = TRUE;
	return S_OK;
}

STDMETHODIMP CCandidateList::FinalizeExactCompositionString()
{
	_tsf._AbortComposition(false);
	return S_OK;
}

void CCandidateList::UpdateUI(const Context& ctx, const Status& status)
{
	if (_ui.style().inline_preedit) {
		_ui.style().client_caps |= weasel::INLINE_PREEDIT_CAPABLE;
	}
	else {
		_ui.style().client_caps &= ~weasel::INLINE_PREEDIT_CAPABLE;
	}

	// In UWP, candidate window will only be shown
	// if it is owned by active view window
	//_UpdateOwner();
	/*if (!ctx.cinfo.candies.empty())
	{
		std::wstring temp{};
		temp.reserve(256);
		temp = std::format(L"{} ", ctx.cinfo.candies[0].str);
		for (int i = 1; i < ctx.cinfo.candies.size(); ++i)
		{
			temp += std::format(L" {}.{} ", i + 1, ctx.cinfo.candies[i].str);
		}
		_pReadingInformation->SetString(temp);		
	}*/

	_ui.Update(ctx, status);
	if (_pbShow == FALSE && status.composing)
	{
		_flags = TF_CLUIE_DOCUMENTMGR | TF_CLUIE_COUNT | TF_CLUIE_SELECTION | TF_CLUIE_STRING | TF_CLUIE_CURRENTPAGE;
		_UpdateUIElement();
	}

	if (status.composing)
	{
		if (_tsf.GetBit(WeaselFlag::WEZTERM_FIRST_KEY))
		{
			_tsf.ResetBit(WeaselFlag::WEZTERM_FIRST_KEY);
			return;
		}
		Show(_pbShow);
	}
	else
	{
		Show(FALSE);
	}
}

void CCandidateList::UpdateStyle(const UIStyle& sty)
{
	_ui.style() = sty;
}

void CCandidateList::UpdateInputPosition(RECT const& rc)
{
	_ui.UpdateInputPosition(rc);
}

void CCandidateList::Destroy()
{
	// EndUI();
	Show(FALSE);
	_DisposeUIWindowAll();
}

UIStyle& CCandidateList::style()
{
	return _style;
}

HWND CCandidateList::_GetActiveWnd()
{
	com_ptr<ITfDocumentMgr> pDocumentMgr;
	com_ptr<ITfContext> pContext;
	com_ptr<ITfContextView> pContextView;
	com_ptr<ITfThreadMgr> pThreadMgr = _tsf._GetThreadMgr();

	HWND w = NULL;

	// Reset current context
	_pContextDocument = nullptr;

	if (pThreadMgr != nullptr
		&& SUCCEEDED(pThreadMgr->GetFocus(&pDocumentMgr))
		&& SUCCEEDED(pDocumentMgr->GetTop(&pContext))
		&& SUCCEEDED(pContext->GetActiveView(&pContextView)))
	{
		// Set current context
		_pContextDocument = pContext;
		pContextView->GetWnd(&w);
	}

	if (w == NULL)
	{
		w = ::GetFocus();
	}
	return w;
}

HRESULT CCandidateList::_UpdateUIElement()
{
	HRESULT hr = S_OK;

	com_ptr<ITfUIElementMgr> pUIElementMgr;
	com_ptr<ITfThreadMgr> pThreadMgr = _tsf._GetThreadMgr();
	if (nullptr == pThreadMgr)
	{
		return S_OK;
	}

	if (SUCCEEDED(pThreadMgr->QueryInterface(&pUIElementMgr)))
	{
		// hr = pUIElementMgr->UpdateUIElement(uiid2);
		hr = pUIElementMgr->UpdateUIElement(uiid);
	}

	return hr;
}

void CCandidateList::StartUI()
{
	com_ptr<ITfThreadMgr> pThreadMgr = _tsf._GetThreadMgr();
	com_ptr<ITfUIElementMgr> pUIElementMgr;
	auto hr = pThreadMgr->QueryInterface(&pUIElementMgr);
	if (FAILED(hr))
		return;

	if (pUIElementMgr == NULL)
	{
		return;
	}

	// ToDo: send select candidate info back to rime	
	hr = pUIElementMgr->BeginUIElement(this, &_pbShow, &uiid);
	// hr = pUIElementMgr->BeginUIElement(_pReadingInformation, &_pbShow, &uiid2);
	if (_tsf.GetBit(WeaselFlag::GAME_MODE_SELF_REDRAW))
		_pbShow = true;
	if (_pbShow)
	{
		if (_tsf.GetBit(WeaselFlag::GAME_MODE))
		{
			_pbShow = false;
			return;
		}
		_ui.style() = _style;
		_MakeUIWindow();
	}
	else
	{
		_tsf.SetBit(WeaselFlag::GAME_MODE);
	}
}

void CCandidateList::EndUI()
{
	if (_tsf.GetBit(WeaselFlag::GAME_MODE))
	{
		com_ptr<ITfThreadMgr> pThreadMgr = _tsf._GetThreadMgr();
		com_ptr<ITfUIElementMgr> emgr;
		if (FAILED(pThreadMgr->QueryInterface(&emgr)))
			return;
		if (emgr != NULL)
		{
			_flags = 0;
			emgr->EndUIElement(uiid);
			// emgr->EndUIElement(uiid2);
			if (_tsf.GetBit(WeaselFlag::GAME_WAR3))
			{
				PostMessage(_GetActiveWnd(), WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 0);
			}
		}
	}
	_DisposeUIWindow();
}

void CCandidateList::SetCaretFollowing(bool following)
{
	_ui.SetCaretFollowing(_tsf.GetBit(WeaselFlag::CARET_FOLLOWING));
}

void CCandidateList::SetThreadFocus()
{
	if (_pbShow)
	{
		Show(TRUE);
	}
}

void CCandidateList::KillThreadFocus()
{
	if (_pbShow)
	{
		Show(FALSE);
	}
}

com_ptr<ITfContext> CCandidateList::GetContextDocument()
{
	return _pContextDocument;
}

bool CCandidateList::GetIsReposition()
{
	return _ui.GetIsReposition();
}

void CCandidateList::_DisposeUIWindow()
{
	_ui.Destroy();
}

void CCandidateList::_DisposeUIWindowAll()
{
	// call _ui.DestroyAll() to clean resources
	_ui.Destroy(true);
}

void CCandidateList::_MakeUIWindow()
{
	HWND p = _GetActiveWnd();
	_ui.Create(p);
}

module;
#include "stdafx.h"
#include <WeaselUI.h>
module ReadingInformation;

CReadingInformation::CReadingInformation(WeaselTSF& textService, weasel::UI* pUI) :
	_tsf{ textService },
	m_pUI{ pUI }
{

}

CReadingInformation::~CReadingInformation()
{
	m_pUI = nullptr;
}

STDMETHODIMP CReadingInformation::QueryInterface(REFIID riid, _Outptr_ void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_INVALIDARG;
	}

	*ppvObj = nullptr;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_ITfUIElement) ||
		IsEqualIID(riid, IID_ITfReadingInformationUIElement))
	{
		*ppvObj = (ITfReadingInformationUIElement*)this;
	}

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CReadingInformation::AddRef(void)
{
	return ++_cRef;
}

STDMETHODIMP_(ULONG) CReadingInformation::Release(void)
{
	LONG cr = --_cRef;

	assert(_cRef >= 0);

	if (_cRef == 0)
	{
		delete this;
	}

	return cr;
}

STDMETHODIMP CReadingInformation::GetDescription(BSTR* pbstr)
{
	static auto str = SysAllocString(L"Reading Information");
	if (pbstr)
	{
		*pbstr = str;
	}
	return S_OK;
}

STDMETHODIMP CReadingInformation::GetGUID(GUID* pguid)
{
	// {6DE7D4A0-8F4A-4480-B7B9-946FDA77C680}
	*pguid = { 0x6de7d4a0, 0x8f4a, 0x4480, { 0xb7, 0xb9, 0x94, 0x6f, 0xda, 0x77, 0xc6, 0x80 } };
	return S_OK;
}

STDMETHODIMP CReadingInformation::Show(BOOL showCandidateWindow)
{
	return S_OK;
}

STDMETHODIMP CReadingInformation::IsShown(BOOL* pIsShow)
{
	if (m_pUI)
		*pIsShow = m_pUI->IsShown();
	return S_OK;
}

STDMETHODIMP CReadingInformation::GetContext(_Outptr_ ITfContext** ppic)
{
	*ppic = nullptr;
	auto pThreadMgr = _tsf._GetThreadMgr();
	if (pThreadMgr == nullptr)
	{
		return E_FAIL;
	}
	com_ptr<ITfDocumentMgr> pDocumentMgr;
	if (SUCCEEDED(pThreadMgr->GetFocus(&pDocumentMgr)) && pDocumentMgr)
	{
		if (FAILED(pDocumentMgr->GetTop(ppic)) || *ppic == nullptr)
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

STDMETHODIMP CReadingInformation::GetErrorIndex(_Outptr_ UINT* pErrorIndex)
{
	return S_OK;
}

STDMETHODIMP CReadingInformation::GetMaxReadingStringLength(_Outptr_ UINT* pcchMax)
{
	if (!pcchMax)
		return E_INVALIDARG;

	*pcchMax = _preedit.size();
	return S_OK;
}

STDMETHODIMP CReadingInformation::GetString(_Outptr_ BSTR* pbstr)
{
	*pbstr = nullptr;
	*pbstr = SysAllocStringLen(_preedit.data(), _preedit.size());
	return S_OK;
}

STDMETHODIMP CReadingInformation::GetUpdatedFlags(_Outptr_ DWORD* pdwFlags)
{
	if (!pdwFlags)
		return E_INVALIDARG;

	*pdwFlags = TF_RIUIE_CONTEXT | TF_RIUIE_STRING | TF_RIUIE_MAXREADINGSTRINGLENGTH | TF_RIUIE_VERTICALORDER;
	return S_OK;
}

STDMETHODIMP CReadingInformation::IsVerticalOrderPreferred(_Outptr_ BOOL* pfVertical)
{
	if (!pfVertical)
		return E_INVALIDARG;

	*pfVertical = TRUE;
	return S_OK;
}

void CReadingInformation::SetString(std::wstring_view str)
{
	_preedit = str.data();
}
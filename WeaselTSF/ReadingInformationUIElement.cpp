module;
#include "stdafx.h"
module ReadingInformationUIElement;


ReadingInformationUIElement::ReadingInformationUIElement()
{
}

ReadingInformationUIElement::~ReadingInformationUIElement()
{
}

STDMETHODIMP ReadingInformationUIElement::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_INVALIDARG;
	}

	*ppvObj = nullptr;

	if (IsEqualIID(riid, IID_ITfReadingInformationUIElement))
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

STDMETHODIMP_(ULONG __stdcall) ReadingInformationUIElement::AddRef(void)
{
	return ++_cRef;
}

STDMETHODIMP_(ULONG __stdcall) ReadingInformationUIElement::Release(void)
{
	LONG cr = --_cRef;
	
	assert(_cRef >= 0);

	if (_cRef == 0)
	{
		delete this;
	}

	return cr;
}

STDMETHODIMP_(HRESULT __stdcall) ReadingInformationUIElement::GetDescription(BSTR* pbstr)
{
	static auto str = SysAllocString(L"Reading Information UI Element");
	if (pbstr)
	{
		*pbstr = str;
	}
	return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall) ReadingInformationUIElement::GetGUID(GUID* pguid)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) ReadingInformationUIElement::Show(BOOL showCandidateWindow)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) ReadingInformationUIElement::IsShown(BOOL* pIsShow)
{
	return E_NOTIMPL;
}
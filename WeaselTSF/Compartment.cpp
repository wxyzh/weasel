module;
#include "stdafx.h"
#include <resource.h>
module Compartment;
import WeaselTSF;
import Compartment;

CCompartment::CCompartment(IUnknown* punk, TfClientId tfClientId, REFGUID guidCompartment)
	: _punk{ punk }, _tfClientId{ tfClientId }, _guidCompartment { guidCompartment }
{
	_punk->AddRef();
}

CCompartment::~CCompartment()
{
	_punk->Release();
}

HRESULT CCompartment::_GetCompartment(ITfCompartment** ppCompartment)
{
	com_ptr<ITfCompartmentMgr> pCompartmentMgr;

	auto hr = _punk->QueryInterface(&pCompartmentMgr);
	if (SUCCEEDED(hr))
	{
		hr = pCompartmentMgr->GetCompartment(_guidCompartment, ppCompartment);
	}

	return hr;
}

HRESULT CCompartment::_GetCompartmentBOOL(BOOL& flag)
{
	HRESULT hr = S_OK;
	com_ptr<ITfCompartment> pCompartment;
	flag = FALSE;

	if ((hr = _GetCompartment(&pCompartment)) == S_OK)
	{
		VARIANT var;
		if ((hr = pCompartment->GetValue(&var)) == S_OK)
		{
			if (var.vt == VT_I4)
			{
				flag = (BOOL)var.lVal; // Even VT_EMPTY, GetValue() can succeed
			}
			else
			{
				hr = S_FALSE;
			}
		}
	}

	return hr;
}

HRESULT CCompartment::_SetCompartmentBOOL(BOOL flag)
{
	com_ptr<ITfCompartment> pCompartment;

	auto hr = _GetCompartment(&pCompartment);
	if (SUCCEEDED(hr))
	{
		VARIANT var;
		var.vt = VT_I4;
		var.lVal = flag;
		hr = pCompartment->SetValue(_tfClientId, &var);
	}

	return hr;
}

HRESULT CCompartment::_GetCompartmentDWORD(_Out_ DWORD& dw)
{
	com_ptr<ITfCompartment> pCompartment;

	auto hr = _GetCompartment(&pCompartment);
	if (SUCCEEDED(hr))
	{
		VARIANT var;
		if ((hr = pCompartment->GetValue(&var)) == S_OK)
		{
			if (var.vt == VT_I4)	// Even VT_EMPTY, GetValue() can succeed
			{
				dw = (DWORD)var.lVal;
			}
			else
			{
				hr = S_FALSE;
			}
		}
	}

	return hr;
}

HRESULT CCompartment::_SetCompartmentDWORD(DWORD dw)
{
	com_ptr<ITfCompartment> pCompartment;

	auto hr = _GetCompartment(&pCompartment);
	if (SUCCEEDED(hr))
	{
		VARIANT var;
		var.vt = VT_I4;
		var.lVal = dw;
		hr = pCompartment->SetValue(_tfClientId, &var);
	}

	return hr;
}

HRESULT CCompartment::_ClearCompartment()
{
	if (IsEqualGUID(_guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
	{
		return S_FALSE;
	}

	com_ptr<ITfCompartmentMgr> pCompartmentMgr;
	HRESULT hr = S_OK;
	if ((hr = _punk->QueryInterface(&pCompartmentMgr)) == S_OK)
	{
		hr = pCompartmentMgr->ClearCompartment(_tfClientId, _guidCompartment);
	}
	
	return hr;
}

STDAPI CCompartmentEventSink::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
	if (ppvObj == nullptr)
		return E_INVALIDARG;

	*ppvObj = nullptr;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_ITfCompartmentEventSink))
	{
		*ppvObj = (CCompartmentEventSink *)this;
	}

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) CCompartmentEventSink::AddRef()
{
	return ++_refCount;
}

STDAPI_(ULONG) CCompartmentEventSink::Release()
{
	LONG cr = --_refCount;

	assert(_refCount >= 0);

	if (_refCount == 0)
	{
		delete this;
	}

	return cr;
}

STDAPI CCompartmentEventSink::OnChange(_In_ REFGUID guidCompartment)
{
	return _callback(guidCompartment);
}

HRESULT CCompartmentEventSink::_Advise(_In_ com_ptr<IUnknown> punk, _In_ REFGUID guidCompartment)
{
	HRESULT hr = S_OK;
	com_ptr<ITfCompartmentMgr> pCompartmentMgr;
	com_ptr<ITfSource> pSource;

	hr = punk->QueryInterface(&pCompartmentMgr);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pCompartmentMgr->GetCompartment(guidCompartment, &_compartment);
	if (SUCCEEDED(hr))
	{
		hr = _compartment->QueryInterface(&pSource);
		if (SUCCEEDED(hr))
		{
			hr = pSource->AdviseSink(IID_ITfCompartmentEventSink, this, &_cookie);
		}
	}

	return hr;
}
HRESULT CCompartmentEventSink::_Unadvise()
{
	HRESULT hr = S_OK;
	com_ptr<ITfSource> pSource;

	hr = _compartment->QueryInterface(&pSource);
	if (SUCCEEDED(hr))
	{
		hr = pSource->UnadviseSink(_cookie);
	}

	_compartment = nullptr;
	_cookie = 0;

	return hr;
}
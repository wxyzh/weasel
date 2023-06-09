#include "stdafx.h"
#include "WeaselTSF.h"
#include "Compartment.h"
#include <resource.h>
#include <functional>

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


BOOL WeaselTSF::_IsKeyboardDisabled()
{
	com_ptr<ITfCompartmentMgr> pCompMgr;
	com_ptr<ITfDocumentMgr> pDocMgrFocus;
	com_ptr<ITfContext> pContext;
	

	if ((_pThreadMgr->GetFocus(&pDocMgrFocus) != S_OK) || (pDocMgrFocus == NULL))
	{
		return TRUE;
	}

	if ((pDocMgrFocus->GetTop(&pContext) != S_OK) || (pContext == NULL))
	{
		return TRUE;
	}

	BOOL fDisabled = FALSE;

	if (pContext->QueryInterface(&pCompMgr) == S_OK)
	{
		com_ptr<ITfCompartment> pCompartmentDisabled;
		com_ptr<ITfCompartment> pCompartmentEmptyContext;

		/* Check GUID_COMPARTMENT_KEYBOARD_DISABLED */
		if (pCompMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_DISABLED, &pCompartmentDisabled) == S_OK)
		{
			VARIANT var;
			if (pCompartmentDisabled->GetValue(&var) == S_OK)
			{
				if (var.vt == VT_I4) // Even VT_EMPTY, GetValue() can succeed
					fDisabled = (BOOL) var.lVal;
			}
		}

		/* Check GUID_COMPARTMENT_EMPTYCONTEXT */
		if (pCompMgr->GetCompartment(GUID_COMPARTMENT_EMPTYCONTEXT, &pCompartmentEmptyContext)  == S_OK)
		{
			VARIANT var;
			if (pCompartmentEmptyContext->GetValue(&var) == S_OK)
			{
				if (var.vt == VT_I4) // Even VT_EMPTY, GetValue() can succeed
					fDisabled = (BOOL) var.lVal;
			}
		}
	}

	return fDisabled;
}

BOOL WeaselTSF::_IsKeyboardOpen()
{
	com_ptr<ITfCompartmentMgr> pCompMgr;
	BOOL fOpen = FALSE;

	if (_pThreadMgr->QueryInterface(&pCompMgr) == S_OK)
	{
		com_ptr<ITfCompartment> pCompartment;
		if (pCompMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
		{
			VARIANT var;
			if (pCompartment->GetValue(&var) == S_OK)
			{
				if (var.vt == VT_I4) // Even VT_EMPTY, GetValue() can succeed
					fOpen = (BOOL) var.lVal;
			}
		}
	}
	return fOpen;
}

HRESULT WeaselTSF::_SetKeyboardOpen(BOOL fOpen)
{
	HRESULT hr = E_FAIL;
	com_ptr<ITfCompartmentMgr> pCompMgr;

	if (_pThreadMgr->QueryInterface(&pCompMgr) == S_OK)
	{
		com_ptr<ITfCompartment> pCompartment;
		if (pCompMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
		{
			VARIANT var;
			var.vt = VT_I4;
			var.lVal = fOpen;
			hr = pCompartment->SetValue(_tfClientId, &var);
		}
	}

	return hr;
}

BOOL WeaselTSF::_InitCompartment()
{
	using namespace std::placeholders;

	auto callback = std::bind(&WeaselTSF::_HandleCompartment, this, _1);
	_pKeyboardCompartmentSink = new CCompartmentEventSink(callback);
	if (!_pKeyboardCompartmentSink)
		return FALSE;
	DWORD hr = _pKeyboardCompartmentSink->_Advise(
		(IUnknown *)_pThreadMgr,
		GUID_COMPARTMENT_KEYBOARD_OPENCLOSE
	);
	return SUCCEEDED(hr);
}

void WeaselTSF::_UninitCompartment()
{
	if (_pKeyboardCompartmentSink) {
		_pKeyboardCompartmentSink->_Unadvise();
		_pKeyboardCompartmentSink = NULL;
	}

}

HRESULT WeaselTSF::_HandleCompartment(REFGUID guidCompartment)
{
	if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
	{
		BOOL isOpen = _IsKeyboardOpen();
		if (isOpen) {
			m_client.TrayCommand(ID_WEASELTRAY_DISABLE_ASCII);
		}
		_EnableLanguageBar(isOpen);
	}
	return S_OK;
}

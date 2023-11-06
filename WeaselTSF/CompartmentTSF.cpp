module;
#include "stdafx.h"
#include "Globals.h"
#include <resource.h>
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;
import Compartment;
import ResponseParser;
import CandidateList;

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
					fDisabled = (BOOL)var.lVal;
			}
		}

		/* Check GUID_COMPARTMENT_EMPTYCONTEXT */
		if (pCompMgr->GetCompartment(GUID_COMPARTMENT_EMPTYCONTEXT, &pCompartmentEmptyContext) == S_OK)
		{
			VARIANT var;
			if (pCompartmentEmptyContext->GetValue(&var) == S_OK)
			{
				if (var.vt == VT_I4) // Even VT_EMPTY, GetValue() can succeed
					fDisabled = (BOOL)var.lVal;
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
					fOpen = (BOOL)var.lVal;
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
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::_SetKeyboardOpen. fOpen = {}, hr = 0x{:X}", fOpen, (unsigned)hr);
#endif // TEST

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
		(IUnknown*)_pThreadMgr,
		GUID_COMPARTMENT_KEYBOARD_OPENCLOSE
	);
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::_InitCompartment. hr = 0x{:X}", (unsigned)hr);
#endif // TEST

	_pConversionCompartmentSink = new CCompartmentEventSink(callback);
	if (!_pConversionCompartmentSink)
		return FALSE;
	hr = _pConversionCompartmentSink->_Advise(
		(IUnknown*)_pThreadMgr,
		GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION
	);
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::_InitCompartment. hr = 0x{:X}", (unsigned)hr);
#endif // TEST

	return SUCCEEDED(hr);
}

void WeaselTSF::_UninitCompartment()
{
	if (_pKeyboardCompartmentSink) {
		_pKeyboardCompartmentSink->_Unadvise();
	}
	if (_pConversionCompartmentSink)
	{
		_pConversionCompartmentSink->_Unadvise();
	}
}

HRESULT WeaselTSF::_HandleCompartment(REFGUID guidCompartment)
{
	if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
	{
		BOOL isOpen = _IsKeyboardOpen();
		if (isOpen) {
			m_client.TrayCommand(ID_WEASELTRAY_DISABLE_ASCII);
			ReSetBit(WeaselFlag::KEYBOARD_DISABLE);	// _bitset[14]: _KeyboardDisable
		}
		else
		{
			SetBit(WeaselFlag::KEYBOARD_DISABLE);	// _bitset[14]: _KeyboardDisable
		}
		_EnableLanguageBar(isOpen);
#ifdef TEST
		LOG(INFO) << std::format("From WeaselTSF::_HandleCompartment. GUID_COMPARTMENT_KEYBOARD_OPENCLOSE: isOpen = {}", isOpen);
#endif // TEST
	}
	else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
	{
		/*weasel::ResponseParser parser(nullptr, nullptr, &_status, nullptr, &_cand->style());
		bool ok = m_client.GetResponseData(std::ref(parser));
		_UpdateLanguageBar(_status);*/
#ifdef TEST
		LOG(INFO) << std::format("From WeaselTSF::_HandleCompartment. GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION");
#endif // TEST
	}
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::_HandleCompartment. guidCompartment = {:X}-{:X}-{:X}-{:X}{:X}{:X}{:X}{:X}{:X}{:X}{:X}", guidCompartment.Data1, guidCompartment.Data2, guidCompartment.Data3, guidCompartment.Data4[0],
		guidCompartment.Data4[1], guidCompartment.Data4[2], guidCompartment.Data4[3], guidCompartment.Data4[4], guidCompartment.Data4[5], guidCompartment.Data4[6], guidCompartment.Data4[7]);
#endif // TEST

	return S_OK;
}

HRESULT WeaselTSF::_GetCompartmentDWORD(_Out_ DWORD& value, const GUID guid)
{
	HRESULT hr = S_OK;
	com_ptr<ITfCompartmentMgr> pCompartmentMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(&pCompartmentMgr)))
	{
		com_ptr<ITfCompartment> pCompartment;
		hr = pCompartmentMgr->GetCompartment(guid, &pCompartment);
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			if ((hr = pCompartment->GetValue(&var)) == S_OK)
			{
				if (var.vt == VT_I4)	// Even VT_EMPTY, GetValue() can succeed
				{
					value = (DWORD)var.lVal;
				}
				else
				{
					hr = S_FALSE;
				}
			}
		}
	}

	return hr;
}

HRESULT WeaselTSF::_SetCompartmentDWORD(DWORD value, const GUID guid)
{
	HRESULT hr = S_OK;
	com_ptr<ITfCompartmentMgr> pCompartmentMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(&pCompartmentMgr)))
	{
		com_ptr<ITfCompartment> pCompartment;
		hr = pCompartmentMgr->GetCompartment(guid, &pCompartment);
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_I4;
			var.lVal = value;
			hr = pCompartment->SetValue(_tfClientId, &var);
		}
	}

	return hr;
}

HRESULT WeaselTSF::_InitGlobalCompartment()
{
	com_ptr<ITfCompartmentMgr> pDaemonCompartment;
	HRESULT ret = _pThreadMgr->GetGlobalCompartment(&pDaemonCompartment);

	if (SUCCEEDED(ret))
	{
		ret = pDaemonCompartment->GetCompartment(WEASEL_COMPARTMENT_GLOAL_DEAMON, &_pGlobalCompartmentDaemon);

		if (SUCCEEDED(ret))
		{
			VARIANT var{};
			ret = _pGlobalCompartmentDaemon->GetValue(&var);

			if (SUCCEEDED(ret))
			{
				if (var.vt == VT_I4)
				{
					if ((var.lVal & 0xFF00) != 0xFC00)
					{
						var.lVal = 0xFC01;
						ret = _pGlobalCompartmentDaemon->SetValue(_tfClientId, &var);

					}
				}
				else
				{
					var.vt = VT_I4;
					var.lVal = 0xFC01;
					ret = _pGlobalCompartmentDaemon->SetValue(_tfClientId, &var);
				}
			}

		}
	}
	return ret;
}
module;
#include "stdafx.h"
#include "Globals.h"
#include <resource.h>
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

	_pConversionCompartmentSink = new CCompartmentEventSink(callback);
	if (!_pConversionCompartmentSink)
		return FALSE;
	hr = _pConversionCompartmentSink->_Advise(
		(IUnknown*)_pThreadMgr,
		GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION
	);

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
			ResetBit(WeaselFlag::KEYBOARD_DISABLE);
		}
		else
		{
			SetBit(WeaselFlag::KEYBOARD_DISABLE);
		}
		_EnableLanguageBar(isOpen);
	}
	else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
	{
	}

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
		ret = pDaemonCompartment->GetCompartment(WEASEL_COMPARTMENT_GLOAL_DEAMON, &_pGlobalCompartment);

		if (SUCCEEDED(ret))
		{
			VARIANT var{};
			var.vt = VT_I4;
			ret = _pGlobalCompartment->GetValue(&var);

			if (unsigned char value{}; SUCCEEDED(ret))
			{
				if (var.vt == VT_I4)
				{
					if ((var.ulVal & 0xFF00'0000) != 0xFC00'0000)
					{
						if (!ReadConfiguration(ConfigFlag::GLOBAL_COMPARTMENT))
							WriteConfiguration(ConfigFlag::GLOBAL_COMPARTMENT);
						var.ulVal = m_globalCompartment;
						ret = _pGlobalCompartment->SetValue(_tfClientId, &var);
						value = static_cast<unsigned char>(m_globalCompartment);
					}
					else
					{
						m_globalCompartment = var.ulVal;
						value = var.bVal;												
					}					
				}
				else
				{
					var.vt = VT_I4;
					if (!ReadConfiguration(ConfigFlag::GLOBAL_COMPARTMENT))
						WriteConfiguration(ConfigFlag::GLOBAL_COMPARTMENT);
					var.ulVal = m_globalCompartment;
					ret = _pGlobalCompartment->SetValue(_tfClientId, &var);
					value = static_cast<unsigned char>(m_globalCompartment);
				}

				SetBit(WeaselFlag::DAEMON_ENABLE, static_cast<bool>(value & 0x1));
				SetBit(WeaselFlag::PRESERVED_KEY_SWITCH, static_cast<bool>(value & 0x2));
			}

		}
	}
	return ret;
}

void WeaselTSF::UpdateGlobalCompartment(bool in)
{
	VARIANT var{};

	if (in)
	{
		var.vt = VT_I4;
		byte v1 = GetBit(WeaselFlag::DAEMON_ENABLE) ? 1 : 0;
		byte v2 = GetBit(WeaselFlag::PRESERVED_KEY_SWITCH) ? 2 : 0;
		var.ulVal = 0xFC00'0000;
		var.ulVal |= v1 | v2;
		m_globalCompartment = var.ulVal;

		_pGlobalCompartment->SetValue(_tfClientId, &var);
		WriteConfiguration(ConfigFlag::GLOBAL_COMPARTMENT);
	}
	else if (SUCCEEDED(_pGlobalCompartment->GetValue(&var)))
	{
		if (var.vt == VT_I4)
		{
			byte value = var.bVal;
			SetBit(WeaselFlag::DAEMON_ENABLE, value & 0x1);
			SetBit(WeaselFlag::PRESERVED_KEY_SWITCH, value & 0x2);
		}
	}
}
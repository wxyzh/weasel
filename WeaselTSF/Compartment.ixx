module;
#include "stdafx.h"
export module Compartment;

export
{
	class CCompartment
	{
	public:
		CCompartment(_In_ IUnknown* punk, TfClientId tfClientId, _In_ REFGUID guidCompartment);
		~CCompartment();

		HRESULT _GetCompartment(_Outptr_ ITfCompartment** ppCompartment);
		HRESULT _GetCompartmentBOOL(_Out_ BOOL& flag);
		HRESULT _SetCompartmentBOOL(_In_ BOOL flag);
		HRESULT _GetCompartmentDWORD(_Out_ DWORD& dw);
		HRESULT _SetCompartmentDWORD(_In_ DWORD dw);
		HRESULT _ClearCompartment();

		VOID _GetGUID(GUID* pguid)
		{
			*pguid = _guidCompartment;
		}

	private:		
		IUnknown* _punk;
		TfClientId _tfClientId;
		GUID _guidCompartment;
	};

	class CCompartmentEventSink : public ITfCompartmentEventSink
	{
	public:
		using Callback = std::function<HRESULT(REFGUID guidCompartment)>;
		CCompartmentEventSink(Callback callback)
			: _callback(callback), _refCount(1) {};
		~CCompartmentEventSink() = default;

		// IUnknown
		STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		// ITfCompartmentEventSink
		STDMETHODIMP OnChange(_In_ REFGUID guid);

		// function
		HRESULT _Advise(_In_ com_ptr<IUnknown> punk, _In_ REFGUID guidCompartment);
		HRESULT _Unadvise();

	private:
		com_ptr<ITfCompartment> _compartment;
		DWORD _cookie;
		Callback _callback;

		LONG _refCount;
	};
}

module;
#include "stdafx.h"
export module ReadingInformationUIElement;

export
{
	class ReadingInformationUIElement : 
		public ITfReadingInformationUIElement
	{
	public:
		ReadingInformationUIElement();
		~ReadingInformationUIElement();

		// IUnknown
		STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		// ITfUIElement
		STDMETHODIMP GetDescription(BSTR* pbstr) override;
		STDMETHODIMP GetGUID(GUID* pguid);
		STDMETHODIMP Show(BOOL showCandidateWindow) override;
		STDMETHODIMP IsShown(BOOL* pIsShow) override;

		// ITfReadingInformationUIElement
		STDMETHODIMP GetContext(_Out_ ITfContext** ppic) override;
		STDMETHODIMP GetErrorIndex(_Out_ UINT* pErrorIndex) override;
		STDMETHODIMP GetMaxReadingStringLength(_Out_ UINT* pcchMax) override;
		STDMETHODIMP GetString(_Out_ BSTR* pstr) override;
		STDMETHODIMP GetUpdatedFlags(_Out_ DWORD* pdwFlags) override;
		STDMETHODIMP IsVerticalOrderPreferred(_Out_ BOOL* pfVertical) override;

	private:
		DWORD _cRef{};
	};
}
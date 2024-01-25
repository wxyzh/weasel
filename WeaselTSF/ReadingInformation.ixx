module;
#include "stdafx.h"
#include <WeaselUI.h>
export module ReadingInformation;
import WeaselTSF;

export
{
	class CReadingInformation :
		public ITfReadingInformationUIElement
	{
	public:
		CReadingInformation(WeaselTSF& textService, weasel::UI* pUI);
		~CReadingInformation();

		// IUnknown
		STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		// ITfUIElement
		STDMETHODIMP GetDescription(BSTR* pbstr) override;
		STDMETHODIMP GetGUID(GUID* pguid);
		STDMETHODIMP Show(BOOL showCandidateWindow) override;
		STDMETHODIMP IsShown(BOOL* pIsShow) override;

		// ITfReadingInformationUIElement methods
		STDMETHODIMP GetContext(_Outptr_ ITfContext** ppic) override;
		STDMETHODIMP GetErrorIndex(_Outptr_ UINT* pErrorIndex) override;
		STDMETHODIMP GetMaxReadingStringLength(_Outptr_ UINT* pcchMax) override;
		STDMETHODIMP GetString(_Outptr_ BSTR* pbstr) override;
		STDMETHODIMP GetUpdatedFlags(_Outptr_ DWORD* pdwFlags) override;		
		STDMETHODIMP IsVerticalOrderPreferred(_Outptr_ BOOL* pfVertical) override;

		void SetString(std::wstring_view str);

	private:
		WeaselTSF& _tsf;
		weasel::UI* m_pUI;
		DWORD _cRef;
		std::wstring _preedit{};
	};
}
module;
#include "stdafx.h"
#include <WeaselCommon.h>
#include "cmath"
#include <UIAutomation.h>
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
export module Composition;
import WeaselTSF;
import EditSession;
import ResponseParser;
import CandidateList;
import WeaselUtility;

export
{
	/* Start Composition */
	class CStartCompositionEditSession : public CEditSession
	{
	public:
		CStartCompositionEditSession(WeaselTSF* pTextService, ITfContext* pContext, bool not_inline_preedit)
			: CEditSession(pTextService, pContext), _not_inline_preedit{ not_inline_preedit }
		{
		}

		/* ITfEditSession */
		STDMETHODIMP DoEditSession(TfEditCookie ec);

	private:
		bool _not_inline_preedit;
	};

	/* End Composition */
	class CEndCompositionEditSession : public CEditSession
	{
	public:
		CEndCompositionEditSession(WeaselTSF* pTextService, ITfContext* pContext, ITfComposition* pComposition, BOOL clear = TRUE)
			: CEditSession(pTextService, pContext), _clear(clear)
		{
			_pComposition = pComposition;
		}

		/* ITfEditSession */
		STDMETHODIMP DoEditSession(TfEditCookie ec);

	private:
		com_ptr<ITfComposition> _pComposition;
		BOOL _clear;
	};

	/* Get Text Extent */
	class CGetTextExtentEditSession : public CEditSession
	{
	public:
		CGetTextExtentEditSession(WeaselTSF* pTextService, ITfContext* pContext, ITfContextView* pContextView, ITfComposition* pComposition)
			: CEditSession(pTextService, pContext), _pContextView{ pContextView }, _pComposition{ pComposition }
		{

		}

		/* ITfEditSession */
		STDMETHODIMP DoEditSession(TfEditCookie ec);

	private:
		com_ptr<ITfContextView> _pContextView;
		com_ptr<ITfComposition> _pComposition;
	};

	/* Inline Preedit */
	class CInlinePreeditEditSession : public CEditSession
	{
	public:
		CInlinePreeditEditSession(WeaselTSF* pTextService, ITfContext* pContext, ITfComposition* pComposition, const std::shared_ptr<weasel::Context> context)
			: CEditSession(pTextService, pContext), _pComposition(pComposition), _context(context)
		{
		}

		/* ITfEditSession */
		STDMETHODIMP DoEditSession(TfEditCookie ec);

	private:
		com_ptr<ITfComposition> _pComposition;
		const std::shared_ptr<weasel::Context> _context;
	};

	/* Update Composition */
	class CInsertTextEditSession : public CEditSession
	{
	public:
		CInsertTextEditSession(WeaselTSF* pTextService, ITfContext* pContext, ITfComposition* pComposition, std::wstring_view text)
			: CEditSession(pTextService, pContext), _text{ text.data() }, _pComposition(pComposition)
		{
		}

		/* ITfEditSession */
		STDMETHODIMP DoEditSession(TfEditCookie ec);

	private:
		std::wstring _text;
		com_ptr<ITfComposition> _pComposition;
	};
}

STDAPI CStartCompositionEditSession::DoEditSession(TfEditCookie ec)
{
	HRESULT hr = E_FAIL;
	com_ptr<ITfInsertAtSelection> pInsertAtSelection;
	com_ptr<ITfRange> pRangeComposition;
	if (FAILED(_pContext->QueryInterface(&pInsertAtSelection)))
		return hr;
	if (FAILED(pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeComposition)))
		return hr;

	com_ptr<ITfContextComposition> pContextComposition;
	com_ptr<ITfComposition> pComposition;
	if (FAILED(_pContext->QueryInterface(&pContextComposition)))
		return hr;
	if (SUCCEEDED(pContextComposition->StartComposition(ec, pRangeComposition, _pTextService, &pComposition))
		&& (pComposition != NULL))
	{
		_pTextService->_SetComposition(pComposition);

		if (_pTextService->GetBit(WeaselFlag::EATEN))	// _bitset[18]: _Eaten
		{
			std::wstring input{ _pTextService->GetInput() };
			hr = pRangeComposition->SetText(ec, 0, input.data(), 1);
			pRangeComposition->Collapse(ec, TF_ANCHOR_END);
		}
		else
		{
			/* WORKAROUND:
			 * CUAS does not provide a correct GetTextExt() position unless the composition is filled with characters.
			 * So we insert a zero width space here.
			 * The workaround is only needed when inline preedit is not enabled.
			 * See https://github.com/rime/weasel/pull/883#issuecomment-1567625762
			 */
			if (_not_inline_preedit && !_pTextService->GetBit(WeaselFlag::ASYNC_EDIT))		// _bitset[15]: _AsyncEdit
			{
				hr = pRangeComposition->SetText(ec, TF_ST_CORRECTION, L"|", 1);
			}

			pRangeComposition->Collapse(ec, TF_ANCHOR_START);
		}
		/* set selection */
		TF_SELECTION tfSelection;
		tfSelection.range = pRangeComposition;
		tfSelection.style.ase = TF_AE_NONE;
		tfSelection.style.fInterimChar = FALSE;
		hr = _pContext->SetSelection(ec, 1, &tfSelection);
	}
#ifdef TEST
	LOG(INFO) << std::format("From CStartCompositionEditSession::DoEditSession. hr = {:#x}", (unsigned)hr);
#endif // TEST

	return hr;
}

STDAPI CEndCompositionEditSession::DoEditSession(TfEditCookie ec)
{
	/* Clear the dummy text we set before, if any. */
	if (_pComposition == nullptr) return S_OK;

	if (_pTextService->GetBit(WeaselFlag::SUPPORT_DISPLAY_ATTRIBUTE))						// _bitset[8]: _SupportDisplayAttribute
	{
		_pTextService->_ClearCompositionDisplayAttributes(ec, _pContext);
	}

	com_ptr<ITfRange> pCompositionRange;
	if (_clear && _pComposition->GetRange(&pCompositionRange) == S_OK)
	{
		pCompositionRange->SetText(ec, 0, L"", 0);
	}

	auto hr = _pComposition->EndComposition(ec);
#ifdef TEST
	LOG(INFO) << std::format("From CEndCompositionEditSession::DoEditSession. hr = {:#x}", (unsigned)hr);
#endif // TEST
	_pTextService->_FinalizeComposition();
	return hr;
}

STDAPI CGetTextExtentEditSession::DoEditSession(TfEditCookie ec)
{
	com_ptr<ITfRange> pRangeComposition;
	RECT rc;
	BOOL fClipped;
	HRESULT hr;

	if (_pComposition != nullptr && SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
	{

	}
	else
	{
		// composition end
		// note: selection.range is always an empty range
		hr = _pContext->GetEnd(ec, &pRangeComposition);
	}

	hr = _pContextView->GetTextExt(ec, pRangeComposition, &rc, &fClipped);
#ifdef TEST
	LOG(INFO) << std::format("From CGetTextExtentEditSession::DoEditSession. rc.left = {}, rc.top = {}, hr = {:#x}, fClipped = {}", rc.left, rc.top, (unsigned)hr, fClipped);
#endif // TEST
	if (hr == 0x80040057)
	{
		_pTextService->SetBit(WeaselFlag::AUTOCAD);							// _bitset[9]:  _AutoCAD
		_pTextService->SetBit(WeaselFlag::NON_DYNAMIC_INPUT);				// _bitset[11]: _NonDynamicInput
		_pTextService->_AbortComposition();
		return hr;
	}

	if (SUCCEEDED(hr) && (rc.left != 0 && rc.top != 0))
	{
		_pTextService->SetRect(rc);

		if (_pTextService->GetBit(WeaselFlag::INLINE_PREEDIT))				// _bitset[5]: _InlinePreedit
		{
			static RECT rcFirst{};
			if (_pTextService->GetBit(WeaselFlag::BEGIN_COMPOSITION))		// _bitset[6]: _BeginComposition
			{
				rcFirst = rc;
				_pTextService->ReSetBit(WeaselFlag::BEGIN_COMPOSITION);		// _bitset[6]: _BeginComposition
			}
			if (_pTextService->GetBit(WeaselFlag::FOCUS_CHANGED))			// _bitset[7]: _FocusChanged
			{
				rcFirst = rc;
				_pTextService->ReSetBit(WeaselFlag::FOCUS_CHANGED);			// _bitset[7]: _FocusChanged
			}
			else if (5 < abs(rcFirst.top - rc.top))
			{
				rcFirst = rc;
			}
			else if (rc.left < rcFirst.left)
			{
				rcFirst = rc;
			}
			else
			{
				rc = rcFirst;
			}
		}
		_pTextService->_SetCompositionPosition(rc);
	}
	else if (_pTextService->GetRect().left != 0)
	{
		_pTextService->_SetCompositionPosition(_pTextService->GetRect());
	}

	return hr;
}

STDAPI CInlinePreeditEditSession::DoEditSession(TfEditCookie ec)
{
	std::wstring preedit = _context->preedit.str;

	com_ptr<ITfRange> pRangeComposition;
	if ((_pComposition->GetRange(&pRangeComposition)) != S_OK)
		return E_FAIL;

	HRESULT hr = pRangeComposition->SetText(ec, 0, preedit.c_str(), preedit.length());
#ifdef TEST
	LOG(INFO) << std::format("From CInlinePreeditEditSession::DoEditSession. preedit = {}, hr = 0x{:X}", to_string(preedit), (unsigned)hr);
#endif // TEST
	if (FAILED(hr))
	{
		_pTextService->SetBit(WeaselFlag::AUTOCAD);						// _bitset[9]:  _AutoCAD
		_pTextService->SetBit(WeaselFlag::NON_DYNAMIC_INPUT);			// _bitset[11]: _NonDynamicInput
		_pTextService->_AbortComposition();
		return hr;
	}

	/* TODO: Check the availability and correctness of these values */
	int sel_cursor{};
	for (size_t i = 0; i < _context->preedit.attributes.size(); i++)
	{
		if (_context->preedit.attributes.at(i).type == weasel::HIGHLIGHTED)
		{
			sel_cursor = _context->preedit.attributes.at(i).range.cursor;
			break;
		}
	}

	if (_pTextService->GetBit(WeaselFlag::SUPPORT_DISPLAY_ATTRIBUTE))	// _bitset[8]: _SupportDisplayAttribute
	{
		_pTextService->_SetCompositionDisplayAttributes(ec, _pContext, pRangeComposition);
	}

	/* Set caret */
	LONG cch;
	TF_SELECTION tfSelection;
	pRangeComposition->ShiftStart(ec, sel_cursor, &cch, nullptr);
	pRangeComposition->Collapse(ec, TF_ANCHOR_START);
	tfSelection.range = pRangeComposition;
	tfSelection.style.ase = TF_AE_NONE;
	tfSelection.style.fInterimChar = FALSE;
	_pContext->SetSelection(ec, 1, &tfSelection);

	return S_OK;
}

STDMETHODIMP CInsertTextEditSession::DoEditSession(TfEditCookie ec)
{
	com_ptr<ITfRange> pRange;
	TF_SELECTION tfSelection;
	HRESULT hRet = S_OK;

	if (FAILED(_pComposition->GetRange(&pRange)))
		return E_FAIL;

	auto ret = pRange->SetText(ec, 0, _text.c_str(), _text.length());
#ifdef TEST
	LOG(INFO) << std::format("From CInsertTextEditSession::DoEditSession. _text = {}, ret = 0x{:X}", to_string(_text, CP_UTF8), (unsigned)ret);
#endif // TEST
	if (FAILED(ret))
	{
		return E_FAIL;
	}

	/* update the selection to an insertion point just past the inserted text. */
	pRange->Collapse(ec, TF_ANCHOR_END);

	tfSelection.range = pRange;
	tfSelection.style.ase = TF_AE_NONE;
	tfSelection.style.fInterimChar = FALSE;

	hRet = _pContext->SetSelection(ec, 1, &tfSelection);

	return hRet;
}
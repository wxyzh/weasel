module;
#include "stdafx.h"
// #include <immdev.h>
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

// #pragma comment(lib, "imm32.lib")

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
		void CalculatePosition(_In_ const RECT& rcExt, _Inout_opt_ RECT& rc);

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
	if (FAILED(hr = _pContext->QueryInterface(&pInsertAtSelection)))
	{
#ifdef TEST
		LOG(INFO) << std::format("From CStartCompositionEditSession::DoEditSession. QueryInterface: hr = 0x{:X}", (unsigned)hr);
#endif // TEST
		return hr;
	}
	if (FAILED(hr = pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeComposition)))
	{
#ifdef TEST
		LOG(INFO) << std::format("From CStartCompositionEditSession::DoEditSession. InsertTextAtSelection: hr = 0x{:X}", (unsigned)hr);
#endif // TEST
		return hr;
	}

	com_ptr<ITfContextComposition> pContextComposition;
	com_ptr<ITfComposition> pComposition;
	if (FAILED(_pContext->QueryInterface(&pContextComposition)))
		return hr;
	if (SUCCEEDED(pContextComposition->StartComposition(ec, pRangeComposition, _pTextService, &pComposition))
		&& (pComposition != NULL))
	{
		_pTextService->_SetComposition(pComposition);

		if (_pTextService->GetBit(WeaselFlag::EATEN))	// 处理数字和标点的合成
		{
			std::wstring input{ _pTextService->GetInput() };
			hr = pRangeComposition->SetText(ec, 0, input.data(), input.size());
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
			if (_not_inline_preedit && !_pTextService->GetBit(WeaselFlag::ASYNC_EDIT) && !_pTextService->GetBit(WeaselFlag::RETRY_COMPOSITION))
			{
				hr = pRangeComposition->SetText(ec, TF_ST_CORRECTION, L" ", 1);
			}
			_pTextService->ResetBit(WeaselFlag::RETRY_COMPOSITION);
			pRangeComposition->Collapse(ec, TF_ANCHOR_START);
		}
		/* set selection */
		TF_SELECTION tfSelection;
		tfSelection.range = pRangeComposition;
		tfSelection.style.ase = TF_AE_NONE;
		tfSelection.style.fInterimChar = FALSE;
		auto ret = _pContext->SetSelection(ec, 1, &tfSelection);
#ifdef TEST
		LOG(INFO) << std::format("From CStartCompositionEditSession::DoEditSession. ret = 0x{:X}, hr = 0x{:X}", (unsigned)ret, (unsigned)hr);
#endif // TEST
	}

	return S_OK;
}

STDAPI CEndCompositionEditSession::DoEditSession(TfEditCookie ec)
{
	/* Clear the dummy text we set before, if any. */
	if (_pComposition == nullptr) return S_OK;

	if (_pTextService->GetBit(WeaselFlag::SUPPORT_DISPLAY_ATTRIBUTE))
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
	LOG(INFO) << std::format("From CEndCompositionEditSession::DoEditSession. hr = 0x{:X}", (unsigned)hr);
#endif // TEST
	_pTextService->_FinalizeComposition();
	return hr;
}

STDAPI CGetTextExtentEditSession::DoEditSession(TfEditCookie ec)
{
	RECT rc{}, rcExt{};
	BOOL fClipped;
	HRESULT hr;
	com_ptr<ITfRange> pRange;

	if (_pComposition == nullptr || FAILED(_pComposition->GetRange(&pRange)))
	{
		// composition end
		// note: selection.range is always an empty range
		hr = _pContext->GetEnd(ec, &pRange);
	}

	HWND hwnd;
	_pContextView->GetWnd(&hwnd);
	_pTextService->SetHWND(hwnd);

	std::wstring name{};
	name.reserve(512);
	GetClassName(hwnd, name.data(), name.capacity());
	name = name.data();

	_pContextView->GetScreenExt(&rcExt);
	hr = _pContextView->GetTextExt(ec, pRange, &rc, &fClipped);

#ifdef TEST
	LOG(INFO) << std::format("From CGetTextExtentEditSession::DoEditSession. rc.left = {}, rc.top = {}, hr = {:#x}, fClipped = {:s}, className = {}, rcExt = ({}, {}, {}, {})",
		rc.left, rc.top, (unsigned)hr, (bool)fClipped, to_string(name, CP_UTF8), rcExt.left, rcExt.top, rcExt.right, rcExt.bottom);
#endif // TEST
	if (hr == 0x80040057)
	{
		_pTextService->OnCompositionTerminated(ec, _pComposition);
		return hr;
	}

	if (SUCCEEDED(hr) && (rc.left != 0 && rc.top != 0))
	{
		CalculatePosition(rcExt, rc);
		_pTextService->_SetCompositionPosition(rc);
#ifdef TEST
		LOG(INFO) << std::format("From CGetTextExtentEditSession::DoEditSession. rc.left = {}, rc.top = {}", rc.left, rc.top);
#endif // TEST
	}
	else if (_pTextService->GetRect().left != 0)						// 个别应用连续输入时，偶尔会获取到原点坐标，此时用最后一次合成的末码坐标替换
	{
		_pTextService->_SetCompositionPosition(_pTextService->GetRect());
	}

	return hr;
}

void CGetTextExtentEditSession::CalculatePosition(const RECT& rcExt, RECT& rc)
{
	bool isIn = PtInRect(&rcExt, POINT(rc.left, rc.top));
	if (isIn)
	{
		// 保存实时坐标，作为下一次坐标的备用方案
		_pTextService->SetRect(rc);
	}
	else if (POINT pt{ _pTextService->GetRect().left, _pTextService->GetRect().top }; PtInRect(&rcExt, pt))
	{
		rc = _pTextService->GetRect();
	}
	else
	{
		rc.right = rc.left = rcExt.left + (rcExt.right - rcExt.left) / 3;
		rc.bottom = rc.top = rcExt.top + (rcExt.bottom - rcExt.top) / 3 * 2;
	}

	if (_pTextService->GetBit(WeaselFlag::INLINE_PREEDIT))				// 仅嵌入式候选框记录首码坐标
	{
		static RECT rcFirst{};
		if (_pTextService->GetBit(WeaselFlag::FIRST_KEY_COMPOSITION))	// 记录首码坐标
		{
			rcFirst = rc;
		}
		else if (_pTextService->GetBit(WeaselFlag::FOCUS_CHANGED))		// 焦点变化时记录坐标
		{
			rcFirst = rc;
			_pTextService->ResetBit(WeaselFlag::FOCUS_CHANGED);
		}
		else if (5 < abs(rcFirst.top - rc.top))							// 个别应用获取坐标时文交替出现Y轴坐标相差±5，需要过滤掉
		{
			rcFirst = rc;
		}
		else if (rc.left < rcFirst.left)								// 换行时更新坐标
		{
			rcFirst = rc;
		}
		else															// 非首码时用首码坐标替换
		{
			rc = rcFirst;
		}
	}
}

STDAPI CInlinePreeditEditSession::DoEditSession(TfEditCookie ec)
{
#ifdef TEST
	LOG(INFO) << std::format("From CInlinePreeditEditSession::DoEditSession.");
#endif // TEST
	std::wstring preedit = _context->preedit.str;

	com_ptr<ITfRange> pRangeComposition;
	if ((_pComposition->GetRange(&pRangeComposition)) != S_OK)
		return E_FAIL;

	com_ptr<ITfProperty> pProperty;
	VARIANT var{};
	_pContext->GetProperty(GUID_PROP_COMPOSING, &pProperty);
	auto ret = pProperty->GetValue(ec, pRangeComposition, &var);

	HRESULT hr = pRangeComposition->SetText(ec, TF_ST_CORRECTION, preedit.c_str(), preedit.length());
#ifdef TEST
	LOG(INFO) << std::format("From CInlinePreeditEditSession::DoEditSession. preedit = {}, hr = 0x{:X}, ret = 0x{:X}, isComposition = {}", to_string(preedit), (unsigned)hr, (unsigned)ret, var.lVal);
#endif // TEST
	if (FAILED(hr))
	{
		// _pTextService->_AbortComposition();
		if (_pTextService->GetBit(WeaselFlag::AUTOCAD))
		{
			_pTextService->ResetBit(WeaselFlag::FOCUS_CHANGED);
			_pTextService->SetBit(WeaselFlag::RETRY_COMPOSITION);
		}
		_pTextService->OnCompositionTerminated(ec, _pComposition);
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

	if (_pTextService->GetBit(WeaselFlag::SUPPORT_DISPLAY_ATTRIBUTE))
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

	if (FAILED(hRet = _pComposition->GetRange(&pRange)))
		return hRet;

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
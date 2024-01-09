module;
#include "stdafx.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;
import Composition;
// import PrivateContextWrapper;

static BOOL IsRangeCovered(TfEditCookie ec, ITfRange* pRangeTest, ITfRange* pRangeCover)
{
	LONG lResult;

	if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) || 0 < lResult)
		return FALSE;
	if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) || lResult < 0)
		return FALSE;

	return TRUE;
}

STDAPI WeaselTSF::OnEndEdit(ITfContext* pContext, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord)
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnEndEdit.");
#endif // TEST
	/* did the selection change? */
	BOOL fSelectionChanged;
	if (SUCCEEDED(pEditRecord->GetSelectionStatus(&fSelectionChanged)) && fSelectionChanged)
	{
		if (_IsComposing())
		{
			/* if the caret moves out of composition range, stop the composition */
			TF_SELECTION tfSelection;
			ULONG cFetched;

			if (SUCCEEDED(pContext->GetSelection(ecReadOnly, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched)) && cFetched == 1)
			{
				com_ptr<ITfRange> pRangeComposition;
				if (_pComposition->GetRange(&pRangeComposition) == S_OK)
				{
					if (!IsRangeCovered(ecReadOnly, tfSelection.range, pRangeComposition))
					{
#ifdef TEST
						LOG(INFO) << std::format("From WeaselTSF::OnEndEdit. The caret moves out of composition range.");
#endif // TEST
						_EndComposition(pContext, true);
					}
				}
			}
		}
	}

	/* text modification? */
	com_ptr<IEnumTfRanges> pEnumTextChanges;	
	if (SUCCEEDED(pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT, NULL, 0, &pEnumTextChanges)))
	{
		com_ptr<ITfRange> pRange;
		ULONG fetched{};
		if (SUCCEEDED(pEnumTextChanges->Next(1, &pRange, &fetched)))
		{
#ifdef TEST
			LOG(INFO) << std::format("From WeaselTSF::OnEndEdit. pEditRecord->GetTextAndPropertyUpdates. fetched = {}", fetched);
#endif // TEST		
			if (GetBit(WeaselFlag::CARET_FOLLOWING) && GetBit(WeaselFlag::FIRST_KEY_COMPOSITION) && fetched == 0)
			{			
				_UpdateCompositionWindow(pContext);
			}
		}
	}
	return S_OK;
}

STDAPI WeaselTSF::OnLayoutChange(ITfContext* pContext, TfLayoutCode lcode, ITfContextView* pContextView)
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnLayoutChange. pContext = 0x{:X}, _pTextEditSinkContext = 0x{:X}, pContextView = 0x{:X}", (size_t)pContext, (size_t)_pTextEditSinkContext.p, (size_t)pContextView);
#endif // TEST
	if (!_IsComposing())
		return S_OK;

	if (pContext != _pTextEditSinkContext)
		return S_OK;

	if (GetBit(WeaselFlag::CARET_FOLLOWING))
	{
		_UpdateCompositionWindow(pContext);
	}

	return S_OK;
}

BOOL WeaselTSF::_InitTextEditSink(ITfDocumentMgr* pDocMgr)
{
	com_ptr<ITfSource> pSource;
	BOOL fRet{};
	HRESULT hr1{}, hr2{};

	/* clear out any previous sink first */
	if (_dwTextEditSinkCookie != TF_INVALID_COOKIE)
	{
		if (SUCCEEDED(_pTextEditSinkContext->QueryInterface(&pSource)))
		{
			hr1 = pSource->UnadviseSink(_dwTextEditSinkCookie);
			hr2 = pSource->UnadviseSink(_dwTextLayoutSinkCookie);
		}
		_pTextEditSinkContext.Release();
		_dwTextEditSinkCookie = TF_INVALID_COOKIE;
		_dwTextLayoutSinkCookie = TF_INVALID_COOKIE;
	}

#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::_InitTextEditSink. pDocMgr = {:#x}, _dwTextEditSinkCookie = {:#x}, hr1 = 0x{:X}, hr2 = 0x{:X}", (size_t)pDocMgr, (unsigned)_dwTextEditSinkCookie, (unsigned)hr1, (unsigned)hr2);
#endif // TEST

	if (pDocMgr == NULL)
		return TRUE;

	if (FAILED(pDocMgr->GetTop(&_pTextEditSinkContext)))
		return FALSE;

	if (_pTextEditSinkContext == NULL)
		return TRUE;

	if (SUCCEEDED(_pTextEditSinkContext->QueryInterface(&pSource)))
	{
		if (SUCCEEDED(pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*)this, &_dwTextEditSinkCookie)))
			fRet = TRUE;
		else
			_dwTextEditSinkCookie = TF_INVALID_COOKIE;
		if (SUCCEEDED(pSource->AdviseSink(IID_ITfTextLayoutSink, (ITfTextLayoutSink*)this, &_dwTextLayoutSinkCookie)))
		{
			fRet = TRUE;
		}
		else
			_dwTextLayoutSinkCookie = TF_INVALID_COOKIE;
	}
	if (fRet == FALSE)
	{
		_pTextEditSinkContext.Release();
	}

	return fRet;
}
module;
#include "stdafx.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;

static BOOL IsRangeCovered(TfEditCookie ec, ITfRange* pRangeTest, ITfRange* pRangeCover)
{
	LONG lResult;

	if (pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult) != S_OK || 0 < lResult)
		return FALSE;
	if (pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult) != S_OK || lResult < 0)
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
	if (pEditRecord->GetSelectionStatus(&fSelectionChanged) == S_OK && fSelectionChanged)
	{
		if (_IsComposing())
		{
			/* if the caret moves out of composition range, stop the composition */
			TF_SELECTION tfSelection;
			ULONG cFetched;

			if (pContext->GetSelection(ecReadOnly, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) == S_OK && cFetched == 1)
			{
				com_ptr<ITfRange> pRangeComposition;
				if (_pComposition->GetRange(&pRangeComposition) == S_OK)
				{
					if (!IsRangeCovered(ecReadOnly, tfSelection.range, pRangeComposition))
					{
						_EndComposition(pContext, true);
					}
				}
			}
		}
	}

	/* text modification? */
	com_ptr<IEnumTfRanges> pEnumTextChanges;
	com_ptr<ITfRange> pRange;
	if (SUCCEEDED(pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT, NULL, 0, &pEnumTextChanges)))
	{
#ifdef TEST
		LOG(INFO) << std::format("From WeaselTSF::OnEndEdit. pEditRecord->GetTextAndPropertyUpdates.");
#endif // TEST		
		if (SUCCEEDED(pEnumTextChanges->Next(1, &pRange, NULL)))
		{
			pRange.Release();
		}
		pEnumTextChanges.Release();

		if (GetBit(17))				// _bitset[17]: _CaretFollowing
		{
			static int count{};
			if (GetBit(13))			// _bitset[13]: _FistKeyComposition
			{
				if (++count == 2)	// _bitset[15]: _AsyncEdit
				{
					count = 0;
					_UpdateCompositionWindow(pContext);
				}
			}
			else
			{
				count = 0;
			}
		}
	}
	return S_OK;
}

STDAPI WeaselTSF::OnLayoutChange(ITfContext* pContext, TfLayoutCode lcode, ITfContextView* pContextView)
{
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::OnLayoutChange. lcode = {:#x}", (unsigned)lcode);
#endif // TEST
	if (!_IsComposing())
		return S_OK;

	if (pContext != _pTextEditSinkContext)
		return S_OK;

	switch (lcode)
	{
	case TF_LC_CREATE:
		break;

	case TF_LC_CHANGE:
		if (GetBit(17))					// _bitset[17]: _CaretFollowing
			_UpdateCompositionWindow(pContext);
		break;

	case TF_LC_DESTROY:
		break;
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

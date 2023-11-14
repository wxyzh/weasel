module;
#include "stdafx.h"
#include "Globals.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;

void WeaselTSF::_ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext* pContext)
{
    com_ptr<ITfRange> pRangeComposition;
    com_ptr<ITfProperty> pDisplayAttributeProperty;

    if (FAILED(_pComposition->GetRange(&pRangeComposition)))
    {
        return;
    }

    if (SUCCEEDED(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty)))
    {
        pDisplayAttributeProperty->Clear(ec, pRangeComposition);
    }
}

BOOL WeaselTSF::_SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext* pContext, ITfRange *pRangeComposition)
{
    com_ptr<ITfProperty> pDisplayAttributeProperty ;
    HRESULT hr = S_OK;

    if (pRangeComposition == nullptr)
        hr = _pComposition->GetRange(&pRangeComposition);
    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = E_FAIL;

    if (SUCCEEDED(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty)))
    {
        VARIANT var;
        var.vt = VT_I4; // we're going to set a TfGuidAtom
        var.lVal = _gaDisplayAttributeInput;

        hr = pDisplayAttributeProperty->SetValue(ec, pRangeComposition, &var);
    }

    // DO NOT release range composition here
    // it will be released in another function
    // pRangeComposition->Release();
    return (hr == S_OK);
}

BOOL WeaselTSF::_InitDisplayAttributeGuidAtom()
{
    com_ptr<ITfCategoryMgr> pCategoryMgr;
    HRESULT hr{ E_FAIL };

    if (SUCCEEDED(_pThreadMgr->QueryInterface(&pCategoryMgr)))
    {
        hr = pCategoryMgr->RegisterGUID(c_guidDisplayAttributeInput, &_gaDisplayAttributeInput);
#ifdef TEST
        LOG(INFO) << std::format("From WeaselTSF::_InitDisplayAttributeGuidAtom. hr = 0x{:X}", (unsigned)hr);
#endif // TEST
    }

    return SUCCEEDED(hr);
}

module;
#include "stdafx.h"
#include "test.h"

#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module TransitoryExtension;

com_ptr<ITfDocumentMgr> TransitoryExtension::ToParentDocumentIfExists(ITfDocumentMgr* pDocumentMgr)
{
	if (pDocumentMgr == nullptr)
	{
		return nullptr;
	}

	com_ptr<ITfContext> pContext;
	if (FAILED(pDocumentMgr->GetTop(&pContext)) || pContext == nullptr)
	{
		return pDocumentMgr;
	}

	TF_STATUS status{};
	if (FAILED(pContext->GetStatus(&status)))
	{
		return pDocumentMgr;
	}

	if ((status.dwStaticFlags & TF_SS_TRANSITORY) != TF_SS_TRANSITORY)
	{
		return pDocumentMgr;
	}

	com_ptr<ITfCompartmentMgr> pCompartmentMgr;
	if (SUCCEEDED(pDocumentMgr->QueryInterface(&pCompartmentMgr)))
	{
		com_ptr<ITfCompartment> pCompartment;
		if (SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_TRANSITORYEXTENSION_PARENT, &pCompartment)))
		{
			VARIANT var{};
			if (SUCCEEDED(pCompartment->GetValue(&var)))
			{
				if (var.vt == VT_UNKNOWN && var.punkVal)
				{
					com_ptr<ITfDocumentMgr> pParentDocumentMgr;
					auto hr = var.punkVal->QueryInterface(&pParentDocumentMgr);
#ifdef TEST
					LOG(INFO) << std::format("From TransitoryExtension::ToParentDocumentIfExists. punkVal = 0x{:X}, hr = 0x{:X}, ret = 0x{:X}", (size_t)var.punkVal, (unsigned)hr);
#endif // TEST
					if (pParentDocumentMgr)
					{
#ifdef TEST
						LOG(INFO) << std::format("From TransitoryExtension::ToParentDocumentIfExists. ok");
#endif // TEST
						return pParentDocumentMgr;
					}
				}
			}
		}
	}

	return pDocumentMgr;
}

com_ptr<ITfContext> TransitoryExtension::ToParentContextIfExists(ITfContext* pContext)
{
	if (pContext == nullptr)
	{
		return nullptr;
	}

	com_ptr<ITfDocumentMgr> pDocumentMgr;
	if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
	{
		com_ptr<ITfContext> pParentContext;
		if (SUCCEEDED(ToParentDocumentIfExists(pDocumentMgr)->GetTop(&pParentContext)))
		{
			if (pParentContext)
			{
#ifdef TEST
				LOG(INFO) << std::format("From TransitoryExtension::ToParentContextIfExists.");
#endif // TEST
				return pParentContext;
			}
		}
	}

	return pContext;
}
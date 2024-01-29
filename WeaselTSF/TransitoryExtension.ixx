#include "stdafx.h"
export module TransitoryExtension;

export
{
	class TransitoryExtension
	{
	public:
		TransitoryExtension() = delete;
		TransitoryExtension(const TransitoryExtension&) = delete;
		TransitoryExtension& operator=(const TransitoryExtension&) = delete;

		static com_ptr<ITfDocumentMgr> ToParentDocumentIfExists(ITfDocumentMgr* pDocumentMgr);
		static com_ptr<ITfContext> ToParentContextIfExists(ITfContext* pContext);
	};
}

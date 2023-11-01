module;
#include "stdafx.h"
#include <WeaselCommon.h>
module Schemata;

using namespace weasel;

Schemata::Schemata(weasel::ResponseParser* pTarget)
	: Deserializer{ pTarget }
{
}

Schemata::~Schemata()
{
}

void Schemata::Store(const weasel::Deserializer::KeyType& key, std::wstring_view value)
{
	if (!m_pTarget->p_schema_list)
		return;
	
	SchemaList& schema_list = *m_pTarget->p_schema_list;
	std::wstringstream ss{ value.data() };
	boost::archive::text_wiarchive ia{ ss };

	ia >> schema_list;
}

Deserializer::Ptr Schemata::Create(ResponseParser* pTarget)
{
	return Deserializer::Ptr{ std::make_shared<Schemata>(pTarget) };
}
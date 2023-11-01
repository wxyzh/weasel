module;
#include "stdafx.h"
export module Schemata;
import Deserializer;

export
{
	class Schemata : public weasel::Deserializer
	{
	public:
		Schemata(weasel::ResponseParser* pTarget);
		virtual ~Schemata();

		// store data
		virtual void Store(const weasel::Deserializer::KeyType& key, std::wstring_view value);
		// factory method
		static weasel::Deserializer::Ptr Create(weasel::ResponseParser* pTarget);
	};
}
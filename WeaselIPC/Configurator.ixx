module;
#include "stdafx.h"
export module Configurator;
import Deserializer;

export
{
	class Configurator : public weasel::Deserializer
	{
	public:
		Configurator(weasel::ResponseParser* pTarget);
		virtual ~Configurator();
		// store data
		virtual void Store(const weasel::Deserializer::KeyType& key, std::wstring const& value) override;
		// factory method
		static weasel::Deserializer::Ptr Create(weasel::ResponseParser* pTarget);
	};
}
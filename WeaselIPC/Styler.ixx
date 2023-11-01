module;
#include "stdafx.h"
export module Styler;
import Deserializer;

export
{
	namespace weasel {
		class Deserializr;
	}

	class Styler : public weasel::Deserializer
	{
	public:
		Styler(weasel::ResponseParser* pTarget);
		virtual ~Styler();
		// store data
		virtual void Store(const weasel::Deserializer::KeyType& key, std::wstring const& value) override;
		// factory method
		static weasel::Deserializer::Ptr Create(weasel::ResponseParser* pTarget);
	};
}
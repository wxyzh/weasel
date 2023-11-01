module;
#include "stdafx.h"
export module Committer;
import Deserializer;

export
{
	class Committer : public weasel::Deserializer
	{
	public:
		Committer(weasel::ResponseParser* pTarget);
		virtual ~Committer();
		// store data
		virtual void Store(const weasel::Deserializer::KeyType& key, std::wstring const& value) override;
		// factory method
		static weasel::Deserializer::Ptr Create(weasel::ResponseParser* pTarget);
	};
}
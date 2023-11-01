module;
#include "stdafx.h"
export module ActionLoader;
import Deserializer;

export
{
	class ActionLoader : public weasel::Deserializer
	{
	public:
		ActionLoader(weasel::ResponseParser* pTarget);
		virtual ~ActionLoader();
		// store data
		virtual void Store(const weasel::Deserializer::KeyType& key, std::wstring const& value) override;
		// factory method
		static weasel::Deserializer::Ptr Create(weasel::ResponseParser* pTarget);
	};
}
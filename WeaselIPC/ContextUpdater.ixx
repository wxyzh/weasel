module;
#include "stdafx.h"
#include <WeaselCommon.h>
export module ContextUpdater;
import Deserializer;

export
{
	class ContextUpdater : public weasel::Deserializer
	{
	public:
		ContextUpdater(weasel::ResponseParser* pTarget);
		virtual ~ContextUpdater();
		virtual void Store(const weasel::Deserializer::KeyType& key, std::wstring const& value) override;

		void _StoreText(weasel::Text& target, Deserializer::KeyType k, std::wstring_view value);
		void _StoreCand(Deserializer::KeyType k, std::wstring_view value);

		static weasel::Deserializer::Ptr Create(weasel::ResponseParser* pTarget);
	};

	class StatusUpdater : public weasel::Deserializer
	{
	public:
		StatusUpdater(weasel::ResponseParser* pTarget);
		virtual ~StatusUpdater();
		virtual void Store(const weasel::Deserializer::KeyType& key, std::wstring const& value) override;

		static weasel::Deserializer::Ptr Create(weasel::ResponseParser* pTarget);
	};
}
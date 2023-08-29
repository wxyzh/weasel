module;
#include "stdafx.h"
export module Deserializer;
import <functional>;
import ResponseParser;

export namespace weasel
{
	class Deserializer
	{
	public:
		using KeyType = std::vector<std::wstring>;
		using Ptr = std::shared_ptr<Deserializer>;
		using Factory = std::function<Ptr (ResponseParser* pTarget)>;

		Deserializer(ResponseParser* pTarget) : m_pTarget(pTarget) {}
		virtual ~Deserializer() {}
		virtual void Store(KeyType const& key, std::wstring const& value) {}

		static void Initialize(ResponseParser* pTarget);
		static void Define(std::wstring const& action, Factory factory);
		static bool Require(std::wstring const& action, ResponseParser* pTarget);

	protected:
		ResponseParser* m_pTarget;

	private:
		static std::map<std::wstring, Factory> s_factories;
	};
}

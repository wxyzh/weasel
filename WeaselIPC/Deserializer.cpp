module;
#include "stdafx.h"
module Deserializer;
import ActionLoader;
import Committer;
import ContextUpdater;
import Configurator;
import Styler;
import ResponseParser;
// import Schemata;

using namespace weasel;


std::map<std::wstring, Deserializer::Factory> Deserializer::s_factories;


void Deserializer::Initialize(ResponseParser* pTarget)
{
	if (s_factories.empty())
	{
		// register factory methods
		// TODO: extend the parser's functionality in the future by defining more actions here
		Define(L"action", ActionLoader::Create);
		Define(L"commit", Committer::Create);
		Define(L"ctx", ContextUpdater::Create);
		Define(L"status", StatusUpdater::Create);
		Define(L"config", Configurator::Create);
		Define(L"style", Styler::Create);
	}

	// loaded by default
	Require(L"action", pTarget);
}

void Deserializer::Define(std::wstring_view action, Factory factory)
{
	s_factories[action.data()] = factory;
	//s_factories.insert(make_pair(action, factory));
}

bool Deserializer::Require(std::wstring_view action, ResponseParser* pTarget)
{
	if (!pTarget)
		return false;

	std::map<std::wstring, Factory>::iterator i = s_factories.find(action.data());
	if (i == s_factories.end())
	{
		// unknown action type
		return false;
	}

	Factory& factory = i->second;

	pTarget->deserializers[action.data()] = factory(pTarget);
	//pTarget->deserializers.insert(make_pair(action, factory(pTarget)));
	return true;
}
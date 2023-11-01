module;
#include "stdafx.h"
module Committer;

using namespace weasel;


Deserializer::Ptr Committer::Create(ResponseParser* pTarget)
{
	return Deserializer::Ptr(new Committer(pTarget));
}

Committer::Committer(ResponseParser* pTarget)
: Deserializer(pTarget)
{
}

Committer::~Committer()
{
}

void Committer::Store(const Deserializer::KeyType& key, std::wstring const& value)
{
	if (!m_pTarget->p_commit)
		return;
	if (key.size() == 1)
		*m_pTarget->p_commit = value;
}

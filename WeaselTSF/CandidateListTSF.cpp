module;
#include "stdafx.h"
module WeaselTSF;
import CandidateList;
import ResponseParser;

using namespace weasel;

void WeaselTSF::_UpdateUI(const Context& ctx, const Status& status)
{
	_cand->UpdateUI(ctx, status);
}

void WeaselTSF::_StartUI()
{
	_cand->StartUI();
}

void WeaselTSF::_EndUI()
{
	_cand->EndUI();
}

void WeaselTSF::_ShowUI()
{
	_cand->Show(TRUE);
}

void WeaselTSF::_HideUI()
{
	_cand->Show(FALSE);
}

com_ptr<ITfContext> WeaselTSF::_GetUIContextDocument()
{
	return _cand->GetContextDocument();
}

void WeaselTSF::_DeleteCandidateList()
{
	_cand->Destroy();
}

void WeaselTSF::InsertText(std::wstring_view str, const size_t index)
{
	m_client.SelectCandidateOnCurrentPage(index);
	// fake a empty presskey to get data back and DoEditSession
	m_client.ProcessKeyEvent(0);
	std::wstring commit;
	weasel::Config config;
	auto context = std::make_shared<weasel::Context>();
	weasel::ResponseParser parser(&commit, context.get(), &_status, &config, &_cand->style());

	auto ok = m_client.GetResponseData(std::ref(parser));

	_UpdateLanguageBar(_status);

	if (ok)
	{
		if (!commit.empty())
		{
			// For auto-selecting, commit and preedit can both exist.
			// Commit and close the original composition first.
			if (!_IsComposing()) 
			{
				_StartComposition(_pEditSessionContext, _fCUASWorkaroundEnabled && !config.inline_preedit);
			}
			_InsertText(_pEditSessionContext, commit);
			_EndComposition(_pEditSessionContext, false);
		}
		if (_status.composing && !_IsComposing())
		{
			_StartComposition(_pEditSessionContext, _fCUASWorkaroundEnabled && !config.inline_preedit);
		}
		else if (!_status.composing && _IsComposing())
		{
			_EndComposition(_pEditSessionContext, true);
		}
		_UpdateCompositionWindow(_pEditSessionContext);
		if (_IsComposing() && config.inline_preedit)
		{
			_ShowInlinePreedit(_pEditSessionContext, context);
		}
	}

	_UpdateUI(*context, _status);
}
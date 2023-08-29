module;
#include "stdafx.h"
module WeaselTSF;
import CandidateList;
import ResponseParser;
import WeaselUtility;
import Compartment;

STDAPI WeaselTSF::DoEditSession(TfEditCookie ec)
{
	// get commit string from server
	std::wstring commit;
	weasel::Config config;
	auto context = std::make_shared<weasel::Context>();
	weasel::ResponseParser parser(&commit, context.get(), &_status, &config, &_cand->style());

	bool ok = m_client.GetResponseData(std::ref(parser));
	_UpdateLanguageBar(_status);

	if (ok)
	{
		if (!commit.empty())
		{
			// For auto-selecting, commit and preedit can both exist.
			// Commit and close the original composition first.
			if (!_IsComposing()) {
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
		if (_IsComposing() && config.inline_preedit)
		{
			_ShowInlinePreedit(_pEditSessionContext, context);
			SetBit(5);					// _bitset[5]: _InlinePreedit
		}
		_UpdateCompositionWindow(_pEditSessionContext);
		SetBit(2, _status.full_shape);	// _bitset[2]:  _full_shape
	}
	_UpdateUI(*context, _status);

	return TRUE;
}


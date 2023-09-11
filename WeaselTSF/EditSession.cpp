module;
#include "stdafx.h"
#include <WeaselCommon.h>
#include "test.h"
#ifdef TEST
#ifdef _M_X64
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif
#endif // TEST
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
	auto state = _UpdateLanguageBar(_status);

#ifdef TEST
#ifdef _M_X64
	LOG(INFO) << std::format("From WeaselTSF::DoEditSession. state = {}, simplication = {}", state, _status.simplication);
#endif // _M_X64
#endif // TEST

	if (ok)
	{
		if ((state || GetBit(12)) && !_IsComposing())	// _bitset[12]: _simplication_state
		{
			_StartComposition(_pEditSessionContext, _fCUASWorkaroundEnabled && !config.inline_preedit);
			_UpdateCompositionWindow(_pEditSessionContext);
			_EndComposition(_pEditSessionContext, true);
			ReSetBit(12);								// _bitset[12]: _simplication_state
		}
		else
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
		}
	}
	_UpdateUI(*context, _status);

	return TRUE;
}


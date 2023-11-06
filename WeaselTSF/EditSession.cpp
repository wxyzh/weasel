module;
#include "stdafx.h"
#include <WeaselCommon.h>
// #include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
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
	LOG(INFO) << std::format("From WeaselTSF::DoEditSession. state = {}, ok = {}", state, ok);
#endif // TEST

	if (GetBit(WeaselFlag::EATEN))		// _bitset[18]: _Eaten
	{
		_StartComposition(_pEditSessionContext, !config.inline_preedit);		
		_EndComposition(_pEditSessionContext, false);
		ReSetBit(WeaselFlag::EATEN);	// _bitset[18]: _Eaten
	}
	else if (ok)
	{
		if (state && !_IsComposing())
		{
			_StartComposition(_pEditSessionContext, !config.inline_preedit);
			if (GetBit(WeaselFlag::CARET_FOLLOWING))				// _bitset[17]: _CaretFollowing
			{
				_UpdateCompositionWindow(_pEditSessionContext);
			}
			else
			{
				_SetCompositionPosition(m_rcFallback);
			}
			_EndComposition(_pEditSessionContext, true);
		}
		else
		{
			if (!commit.empty())
			{
				// For auto-selecting, commit and preedit can both exist.
				// Commit and close the original composition first.
				if (!_IsComposing()) {
					_StartComposition(_pEditSessionContext, !config.inline_preedit);
				}
				_InsertText(_pEditSessionContext, commit);
				_EndComposition(_pEditSessionContext, false);

				if (GetBit(WeaselFlag::CLEAR_CANDIDATES))
				{
					ReSetBit(WeaselFlag::CLEAR_CANDIDATES);
					unsigned send{};
					std::array<INPUT, 4> inputs;

					inputs[0].type = INPUT_KEYBOARD;
					inputs[0].ki = { 0x41, 0x41, 0, 0, 0 };

					inputs[1].type = INPUT_KEYBOARD;
					inputs[1].ki = { 0x41, 0x41, KEYEVENTF_KEYUP, 0, 0 };

					inputs[2].type = INPUT_KEYBOARD;
					inputs[2].ki = { VK_BACK, 0x08, 0, 0, 0 };

					inputs[3].type = INPUT_KEYBOARD;
					inputs[3].ki = { VK_BACK, 0x08, KEYEVENTF_KEYUP, 0, 0 };
					send = SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
				}
			}
			if (_status.composing && !_IsComposing())
			{
				_StartComposition(_pEditSessionContext, !config.inline_preedit);
			}
			else if (!_status.composing && _IsComposing())
			{
				_EndComposition(_pEditSessionContext, true);
			}
			if (_IsComposing() && config.inline_preedit)
			{
				_ShowInlinePreedit(_pEditSessionContext, context);
				SetBit(WeaselFlag::INLINE_PREEDIT);					// _bitset[5]: _InlinePreedit
			}
			if (GetBit(WeaselFlag::CARET_FOLLOWING))				// _bitset[17]: _CaretFollowing
			{
				_UpdateCompositionWindow(_pEditSessionContext);
			}
			else
			{
				_SetCompositionPosition(m_rcFallback);
			}
		}
	}		
	_UpdateUI(*context, _status);

	return S_OK;
}


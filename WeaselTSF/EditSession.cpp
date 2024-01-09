module;
#include "stdafx.h"
#include <WeaselCommon.h>
#include "test.h"
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
	// auto context = std::make_shared<weasel::Context>();
	weasel::ResponseParser parser(&commit, m_context.get(), &_status, &config, &_cand->style());

	bool ok = m_client.GetResponseData(std::ref(parser));
	auto state = _UpdateLanguageBar();

#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::DoEditSession. state = {}, ok = {}, isComposing = {}, prediction = {:s}, _status.prediction = {:s}, s2t = {:s}", state, ok, _IsComposing(), GetBit(WeaselFlag::PREDICTION), _status.prediction, _status.s2t);
#endif // TEST

	if (GetBit(WeaselFlag::EATEN))				// Word 2021进入异步编辑时，遇到标点符号输入时，必须进入合成，否则句点输入就失去了数字后的智能判断
	{
		_StartComposition(_pEditSessionContext, !config.inline_preedit);		
		_EndComposition(_pEditSessionContext, false);
		ResetBit(WeaselFlag::EATEN);
	}
	else if (ok)
	{
		if (state && !_IsComposing())			// 执行算法服务内的UI状态更新，为它提供正确的坐标
		{
			_StartComposition(_pEditSessionContext, !config.inline_preedit);
			if (GetBit(WeaselFlag::CARET_FOLLOWING))
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
			m_preedit = !m_context->preedit.empty();
			if (!commit.empty())
			{
				// For auto-selecting, commit and preedit can both exist.
				// Commit and close the original composition first.
				if (!_IsComposing())
				{
					_StartComposition(_pEditSessionContext, !config.inline_preedit);
				}
				_InsertText(_pEditSessionContext, commit);
				_EndComposition(_pEditSessionContext, false);
			}
			if (_status.composing && !_IsComposing())
			{
				_StartComposition(_pEditSessionContext, !config.inline_preedit);
			}
			else if (!_status.composing && _IsComposing())
			{
				_EndComposition(_pEditSessionContext, true);
			}
			if (_IsComposing() && (config.inline_preedit || GetBit(WeaselFlag::GAME_MODE)))
			{
				_ShowInlinePreedit(_pEditSessionContext, m_context);
				SetBit(WeaselFlag::INLINE_PREEDIT);
			}
			if (GetBit(WeaselFlag::CARET_FOLLOWING))
			{
				_UpdateCompositionWindow(_pEditSessionContext);
			}
			else
			{
				_SetCompositionPosition(m_rcFallback);
			}
		}
	}		
	_UpdateUI(*m_context, _status);

	return S_OK;
}


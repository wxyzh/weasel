module;
#include "stdafx.h"
module WeaselTSF;
import CandidateList;
import ResponseParser;
import KeyEvent;

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

void WeaselTSF::_SelectCandidateOnCurrentPage(const int index)
{
	m_client.SelectCandidateOnCurrentPage(index);
	// fake a empty presskey to get data back and DoEditSession
	m_client.ProcessKeyEvent(0);
	_UpdateComposition(_pEditSessionContext);
}

void WeaselTSF::_HandleMouseHoverEvent(const int index)
{
	// ToDo: if feature new api comes, replae the processes bellow
	int current_select{};
	_cand->GetSelection(reinterpret_cast<UINT*>(&current_select));
	weasel::KeyEvent ke{ 0, 0 };
	ke.keycode = current_select < index ? ibus::Down : ibus::Up;

	int inc = index > current_select ? 1 : (index < current_select) ? -1 : 0;
	if (_cand->GetIsReposition())
	{
		inc = -inc;
	}
	if (index != current_select)
	{
		for (int i{}; i < abs(index - current_select); ++i)
		{
			_cand->SetSelection(current_select + inc);
			m_client.ProcessKeyEvent(ke);
			_UpdateComposition(_pEditSessionContext);
		}
	}
}

void WeaselTSF::_HandleMousePageEvent(const bool nextPage)
{
	// ToDo: if feature new api comes, replace the processes bellow
	weasel::KeyEvent ke{ 0, 0 };
	if (_cand->style().paging_on_scroll)
	{
		ke.keycode = nextPage ? ibus::Page_Down : ibus::Page_Up;
	}
	else
	{
		ke.keycode = nextPage ? ibus::Down : ibus::Up;
		if (_cand->GetIsReposition())
		{
			if (ke.keycode == ibus::Up)
			{
				ke.keycode = ibus::Down;
			}
			else if (ke.keycode == ibus::Down)
			{
				ke.keycode = ibus::Up;
			}
		}
	}
	m_client.ProcessKeyEvent(ke);
	_UpdateComposition(_pEditSessionContext);
}

void WeaselTSF::HandleUICallback(int* const sel, int* const hov, bool* const next)
{
	if (sel)
	{
		_SelectCandidateOnCurrentPage(*sel);
	}
	if (hov)
	{
		_HandleMouseHoverEvent(*hov);
	}
	if (next)
	{
		_HandleMousePageEvent(*next);
	}
}
#include "pch.h"
#include <WeaselUI.h>
#include "WeaselPanel.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#else
#include "logging.h"
#endif // TEST

using namespace weasel;

class UI::UIImpl
{
public:
	WeaselPanel panel;

	UIImpl(weasel::UI& ui)
		: panel(ui), shown(false)
	{
	}
	~UIImpl()
	{
	}
	void Refresh()
	{
		if (!panel.IsWindow()) return;
		if (timer)
		{
			Hide();
			KillTimer(panel.m_hWnd, AUTOHIDE_TIMER);
			timer = 0;
		}
		panel.Refresh();
	}
	void Show();
	void Hide();
	void ShowWithTimeout(DWORD millisec);
	bool IsShown() const { return shown; }

	static VOID CALLBACK OnTimer(
		_In_  HWND hwnd,
		_In_  UINT uMsg,
		_In_  UINT_PTR idEvent,
		_In_  DWORD dwTime
	);
	static const int AUTOHIDE_TIMER = 20121220;
	static UINT_PTR timer;
	bool shown;
};

UINT_PTR UI::UIImpl::timer = 0;

void UI::UIImpl::Show()
{
#ifdef TEST
	LOG(INFO) << std::format("From UIImpl::Show. panel.IsWindow() = {}", panel.IsWindow());
#endif // TEST
	if (!panel.IsWindow()) return;
	panel.ShowWindow(SW_SHOWNA);
	shown = true;
	if (timer)
	{
		KillTimer(panel.m_hWnd, AUTOHIDE_TIMER);
		timer = 0;
	}
}

void UI::UIImpl::Hide()
{
	if (!panel.IsWindow()) return;
	panel.ShowWindow(SW_HIDE);
	shown = false;
	if (timer)
	{
		KillTimer(panel.m_hWnd, AUTOHIDE_TIMER);
		timer = 0;
	}
}

void UI::UIImpl::ShowWithTimeout(DWORD millisec)
{
	if (!panel.IsWindow()) return;
	DLOG(INFO) << "ShowWithTimeout: " << millisec;
	panel.ShowWindow(SW_SHOWNA);
	shown = true;
	SetTimer(panel.m_hWnd, AUTOHIDE_TIMER, millisec, &UIImpl::OnTimer);
	timer = UINT_PTR(this);
}
VOID CALLBACK UI::UIImpl::OnTimer(
  _In_  HWND hwnd,
  _In_  UINT uMsg,
  _In_  UINT_PTR idEvent,
  _In_  DWORD dwTime
)
{
	DLOG(INFO) << "OnTimer:";
	KillTimer(hwnd, idEvent);
	UIImpl* self = (UIImpl*)timer;
	timer = 0;
	if (self)
	{
		self->Hide();
		self->shown = false;
	}
}

UI::UI()
{
	pimpl_ = nullptr;
}

UI::~UI()
{
	if (pimpl_)
		Destroy(true);
	if (pDWR_)
	{
		pDWR_ = nullptr;
	}
}

bool UI::Create(HWND parent)
{
#ifdef TEST
	LOG(INFO) << std::format("From UI::Create.");
#endif // TEST
	if (!pimpl_)
		pimpl_ = std::make_unique<UIImpl>(*this);

	pimpl_->panel.Create(parent, 0, 0, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT, 0U, 0);
	return true;
}

// for ending of composition
void UI::Destroy(bool full)
{
	if (pimpl_)
	{
		// destroy panel
		if (pimpl_->panel.IsWindow())
		{
			pimpl_->panel.DestroyWindow();
#ifdef TEST
			LOG(INFO) << std::format("From UI::Destroy. pimpl_->panel.DestroyWindow()");
#endif // TEST
		}
		if (full)
		{
			pimpl_ = nullptr;
			pDWR_.reset();
		}
	}
}

void UI::Show()
{
#ifdef TEST
	LOG(INFO) << std::format("From UI::Show.");
#endif // TEST
	if (pimpl_)
	{
		pimpl_->Show();
	}
}

void UI::Hide()
{
	if (pimpl_)
	{
		pimpl_->Hide();
	}
}

void UI::ShowWithTimeout(DWORD millisec)
{
	if (pimpl_)
	{
		pimpl_->ShowWithTimeout(millisec);
	}
}

bool UI::IsCountingDown() const
{
	return pimpl_ && pimpl_->timer != 0;
}

bool UI::IsShown() const
{
	return pimpl_ && pimpl_->IsShown();
}

void UI::Refresh()
{
	if (pimpl_)
	{
		pimpl_->Refresh();
	}
}

void UI::UpdateInputPosition(RECT const& rc)
{
	if (pimpl_ && pimpl_->panel.IsWindow())
	{
		pimpl_->panel.MoveTo(rc);
	}
}

void UI::Update(const Context &ctx, const Status &status)
{
	ctx_ = ctx;
	status_ = status;
	Refresh();
}

bool weasel::UI::GetIsReposition()
{
	if (pimpl_)
		return pimpl_->panel.GetIsReposition();
	return false;
}

void weasel::UI::SetCaretFollowing(const bool following)
{
	_CaretFollowing = following;
	if (pimpl_)
		pimpl_->panel.SetCaretFollowing(following);
}

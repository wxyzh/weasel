#include "stdafx.h"
#include "resource.h"
#include "ConfiguratorDialog.h"

const double AdjustParam{ 1.6 };
const double AdjustParam2{ 0.6 };

BOOL ConfiguratorDialog::PreTranslateMessage(MSG* pMsg)
{
	return 0;
}

BOOL ConfiguratorDialog::OnIdle()
{
	return 0;
}

ConfiguratorDialog::ConfiguratorDialog(RimeSwitcherSettings& switcher, UIStyleSettings& style) :
	m_switcher{ switcher },
	m_style{ style }
{
}

LRESULT ConfiguratorDialog::OnInitDialog(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	// center the dialog on the screen
	CDialogImpl::CenterWindow();

	m_tab.Attach(GetDlgItem(IDC_TAB));
	m_tab.AddItem(L"方案");
	m_tab.AddItem(L"样式");
	// m_tab.SetExtendedStyle(0, TCS_VERTICAL);

	// GetDpiForWindow()
	RECT rc, rc1;
	GetClientRect(&rc);
	m_tab.GetItemRect(0, &rc1);
	rc.top += rc1.bottom * AdjustParam;
	rc.bottom -= rc.bottom * AdjustParam2;

	m_switcherSettings.Create(m_hWnd, lParam);
	m_switcherSettings.SetWindowPos(nullptr, &rc, SWP_SHOWWINDOW);

	m_styleSettings.Create(m_hWnd, lParam);
	m_styleSettings.SetWindowPos(nullptr, &rc, SWP_HIDEWINDOW);

	return LRESULT();
}

LRESULT ConfiguratorDialog::OnCancel(WORD, WORD wID, HWND, BOOL&)
{

	CloseDialog(wID);
	return 0;
}

LRESULT ConfiguratorDialog::OnOK(WORD, WORD wID, HWND, BOOL&)
{
	CloseDialog(wID);
	return 0;
}

void ConfiguratorDialog::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

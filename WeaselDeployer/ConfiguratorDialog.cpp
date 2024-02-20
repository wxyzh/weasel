#include "stdafx.h"
#include "resource.h"
#include <WeaselVersion.h>
#include "WeaselDeployer.h"
#include "UIStyleSettings.h"
#include <rime_api.h>
#include <rime_levers_api.h>
#include "ConfiguratorDialog.h"
#include <dwmapi.h>
#pragma warning(default: 4005)

#pragma comment(lib, "Dwmapi.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

const double AdjustParam{ 1.6 };

BOOL ConfiguratorDialog::OnIdle()
{
	UIUpdateChildWindows();
	return 0;
}

ConfiguratorDialog::ConfiguratorDialog(RimeSwitcherSettings* switcher_settings, UIStyleSettings* ui_style_settings) :

	m_switcherSettings{ switcher_settings, m_selected },
	m_styleSettings{ ui_style_settings }
{
}

LRESULT ConfiguratorDialog::OnInitDialog(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	BOOL value{ TRUE };
	::DwmSetWindowAttribute(m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

	DWM_SYSTEMBACKDROP_TYPE systemBackDrop{ DWMSBT_TABBEDWINDOW };
	::DwmSetWindowAttribute(m_hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &systemBackDrop, sizeof(DWM_SYSTEMBACKDROP_TYPE));

	// center the dialog on the screen
	CDialogImpl::CenterWindow();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);

	m_tab.Attach(GetDlgItem(IDC_TAB));
	auto hFont = m_font.CreatePointFont(110, L"@Segoe UI");
	m_tab.SetFont(hFont);
	m_tab.AddItem(L"方案");
	m_tab.AddItem(L"样式");
	m_tab.ModifyStyle(0, TCS_FIXEDWIDTH | TCS_MULTILINE | WS_VISIBLE);
	// m_tab.ModifyStyleEx(0, DT_VCENTER | DT_CENTER, SWP_SHOWWINDOW);

	// GetDpiForWindow()
	RECT rc, rc1;
	GetClientRect(&rc);
	m_tab.GetItemRect(0, &rc1);
	SIZE size{ rc1.bottom / 3 * 2, rc1.right - rc1.left };
	m_tab.SetItemSize(size);

	// rc.left += rc1.right * AdjustParam;

	// MessageBox(std::format(L"left = {}, right = {}, top = {}, bottom = {}", rc1.left, rc1.right, rc1.top, rc1.bottom).data());

	m_styleSettings.Create(m_hWnd, lParam);
	m_styleSettings.SetWindowPos(nullptr, &rc, SWP_HIDEWINDOW | SWP_NOSIZE);

	m_switcherSettings.Create(m_hWnd, lParam);
	m_switcherSettings.SetWindowPos(nullptr, &rc, SWP_SHOWWINDOW | SWP_NOSIZE);

	m_switcherSettings.ShowWindow(SW_SHOWDEFAULT);

	InitDynamicLayout();

	return TRUE;
}

LRESULT ConfiguratorDialog::OnCancel(WORD, WORD wID, HWND, BOOL&)
{
	m_switcherSettings.CloseDialog(0);
	m_styleSettings.CloseDialog(0);
	EndDialog(wID);
	return 0;
}

LRESULT ConfiguratorDialog::OnOK(WORD code, WORD wID, HWND hwnd, BOOL& handled)
{
	// MessageBox(L"OK");
	m_switcherSettings.OnOK(code, wID, hwnd, handled);
	if (!m_selected)
	{		
		m_styleSettings.ShowWindow(SW_HIDE);
		m_tab.SetCurSel(0);
		m_switcherSettings.ShowWindow(SW_SHOWDEFAULT);
		MessageBox(L"至少要选用一项吧。", L"小狼毫不是这般用法", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	m_styleSettings.CloseDialog(0);
	EndDialog(wID);
	return 0;
}

LRESULT ConfiguratorDialog::OnSelChange(int, LPNMHDR, BOOL&)
{
	auto index = m_tab.GetCurSel();
	switch (index)
	{
	case 0:
		m_switcherSettings.ShowWindow(SW_SHOW);
		m_styleSettings.ShowWindow(SW_HIDE);
		break;

	case 1:
		m_switcherSettings.ShowWindow(SW_HIDE);
		m_styleSettings.ShowWindow(SW_SHOW);
		break;
	}
	return 0;
}

LRESULT ConfiguratorDialog::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc{}, rc1{};
	GetClientRect(&rc);
	m_tab.GetItemRect(0, &rc1);
	// rc.left += rc1.right * AdjustParam;
	m_switcherSettings.SetWindowPos(nullptr, &rc, 0);
	m_styleSettings.SetWindowPos(nullptr, &rc, 0);

	return CDynamicDialogLayout::OnSize(uMsg, wParam, lParam, bHandled);
}

void ConfiguratorDialog::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

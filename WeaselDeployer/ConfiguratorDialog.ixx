module;
#include "stdafx.h"
#include "resource.h"
#include <WeaselVersion.h>
#include "WeaselDeployer.h"
#include <rime_api.h>
#include <rime_levers_api.h>
#pragma warning(default: 4005)
export module ConfiguratorDialog;
import SwitcherSettingsDialog;
import UIStyleSettingsDialog;
import UIStyleSettings;

export
{
	class ConfiguratorDialog :
		public CDialogImpl<ConfiguratorDialog>,
		public CUpdateUI<ConfiguratorDialog>,
		public CDynamicDialogLayout<ConfiguratorDialog>
	{
	public:
		enum { IDD = IDD_CONFIGURATOR };

		virtual BOOL OnIdle();

		BEGIN_UPDATE_UI_MAP(CMainDlg)
		END_UPDATE_UI_MAP()

		ConfiguratorDialog(RimeSwitcherSettings* switcher_settings, UIStyleSettings* ui_style_settings);

		BEGIN_MSG_MAP(ConfiguratorDialog)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
			COMMAND_ID_HANDLER(IDOK, OnOK)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			NOTIFY_ID_HANDLER(IDC_TAB, OnSelChange)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnCancel(WORD, WORD wID, HWND, BOOL&);
		LRESULT OnOK(WORD, WORD, HWND, BOOL&);
		LRESULT OnSelChange(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		void CloseDialog(int nVal);

	private:
		bool m_selected{ true };
		CFont m_font;
		CTabCtrl m_tab;
		SwitcherSettingsDialog m_switcherSettings;
		UIStyleSettingsDialog m_styleSettings;
	};
}

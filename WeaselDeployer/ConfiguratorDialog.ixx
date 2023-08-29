module;
#include "stdafx.h"
#include "resource.h"
#include <WeaselVersion.h>
#include "WeaselDeployer.h"
#pragma warning(disable: 4005)
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
		public CMessageFilter,
		public CIdleHandler
	{
	public:
		enum { IDD = IDD_CONFIGURATOR };

		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual BOOL OnIdle();

		ConfiguratorDialog(RimeSwitcherSettings& switcher, UIStyleSettings& style);

		BEGIN_MSG_MAP(ConfiguratorDialog)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
			COMMAND_ID_HANDLER(IDOK, OnOK)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnCancel(WORD, WORD wID, HWND, BOOL&);
		LRESULT OnOK(WORD, WORD, HWND, BOOL&);

		void CloseDialog(int nVal);

	private:
		CTabCtrl m_tab;
		RimeSwitcherSettings& m_switcher;
		UIStyleSettings& m_style;
		SwitcherSettingsDialog m_switcherSettings{ &m_switcher };
		UIStyleSettingsDialog m_styleSettings{ &m_style };
	};
}
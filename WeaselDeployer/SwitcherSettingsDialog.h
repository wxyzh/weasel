#pragma once

class SwitcherSettingsDialog :
	public CDialogImpl<SwitcherSettingsDialog>
{
public:
	enum { IDD = IDD_SWITCHER_SETTING };

	BEGIN_UPDATE_UI_MAP(SwitcherSettingsDialog)
	END_UPDATE_UI_MAP()

	SwitcherSettingsDialog(RimeSwitcherSettings* settings, bool& selected);
	~SwitcherSettingsDialog();

protected:
	BEGIN_MSG_MAP(SwitcherSettingsDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_GET_SCHEMATA, BN_CLICKED, OnGetSchemata)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		NOTIFY_HANDLER(IDC_SCHEMA_LIST, LVN_ITEMCHANGED, OnSchemaListItemChanged)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnGetSchemata(WORD, WORD, HWND, BOOL&);
	LRESULT OnOK(WORD, WORD, HWND, BOOL&);
	LRESULT OnSchemaListItemChanged(int, LPNMHDR, BOOL&);

	void Populate();
	void ShowDetails(RimeSchemaInfo* info);
	void CloseDialog(int nVal);
	void GetSchemata(HWND hWndCtl);

	RimeLeversApi* api_;
	RimeSwitcherSettings* settings_;
	bool loaded_;
	bool modified_;

	CCheckListViewCtrl schema_list_;
	CStatic description_;
	CEdit hotkeys_;
	CButton get_schemata_;

private:
	bool& m_selected;
};
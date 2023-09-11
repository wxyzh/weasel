module;
#include "stdafx.h"
#include "resource.h"
#include "WeaselDeployer.h"
#include <rime_api.h>
#include <rime_levers_api.h>
#include "DictManagementDialog.h"
#include <dwmapi.h>
import Config;
import WeaselUtility;

#pragma comment(lib, "Dwmapi.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

DictManagementDialog::DictManagementDialog()
{
	api_ = (RimeLeversApi*)rime_get_api()->find_module("levers")->get_api();
}

DictManagementDialog::~DictManagementDialog()
{
	g_hwnd = nullptr;
}

void DictManagementDialog::Populate() {
	RimeUserDictIterator iter = {0};
	api_->user_dict_iterator_init(&iter);
	while (const char* dict = api_->next_user_dict(&iter)) {
		user_dict_list_.AddString(to_wstring(dict, CP_UTF8).data());
	}
	api_->user_dict_iterator_destroy(&iter);
	user_dict_list_.SetCurSel(-1);
}

LRESULT DictManagementDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	BOOL value{ TRUE };
	::DwmSetWindowAttribute(m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
	user_dict_list_.Attach(GetDlgItem(IDC_USER_DICT_LIST));
	backup_.Attach(GetDlgItem(IDC_BACKUP));
	backup_.EnableWindow(FALSE);
	restore_.Attach(GetDlgItem(IDC_RESTORE));
	restore_.EnableWindow(TRUE);
	export_.Attach(GetDlgItem(IDC_EXPORT));
	export_.EnableWindow(FALSE);
	import_.Attach(GetDlgItem(IDC_IMPORT));
	import_.EnableWindow(FALSE);
	
	Populate();
	
	CenterWindow();
	BringWindowToTop();
	g_hwnd = m_hWnd;
	return TRUE;
}

LRESULT DictManagementDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&) {
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT DictManagementDialog::OnBackup(WORD, WORD code, HWND, BOOL&) {
	int sel = user_dict_list_.GetCurSel();
	if (sel < 0 || sel >= user_dict_list_.GetCount()) {
		MessageBox(L"请在左列选择要导出的词典名称。", L":-(", MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	std::wstring path;
	{
		char dir[MAX_PATH] = {0};
		rime_get_api()->get_user_data_sync_dir(dir, _countof(dir));
		WCHAR wdir[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, dir, -1, wdir, _countof(wdir));
		path = wdir;
	}
	if (_waccess_s(path.c_str(), 0) != 0 &&
		!CreateDirectoryW(path.c_str(), NULL) &&
		GetLastError() == ERROR_PATH_NOT_FOUND) {
		MessageBox(L"未能完成导出操作。会不会是同步文件夹无法访问？", L":-(", MB_OK | MB_ICONERROR);
		return 0;
	}
	WCHAR dict_name[100] = {0};
	user_dict_list_.GetText(sel, dict_name);
	path += std::format(LR"(\{}.userdb.txt)", dict_name);
	if (!api_->backup_user_dict(to_string(dict_name, CP_UTF8).data())) {
		MessageBox(L"不知哪里出错了，未能完成导出操作。", L":-(", MB_OK | MB_ICONERROR);
		return 0;
	}
	else if (_waccess(path.c_str(), 0) != 0) {
		MessageBox(L"咦，输出的快照文件找不着了。", L":-(", MB_OK | MB_ICONERROR);
		return 0;
	}
	std::wstring param{ std::format(LR"(/select, "{}")", path) };
	ShellExecute(NULL, L"open", L"explorer.exe", param.data(), NULL, SW_SHOWNORMAL);
	return 0;
}

LRESULT DictManagementDialog::OnRestore(WORD, WORD code, HWND, BOOL&) {
	CFileDialog dlg(TRUE, L"snapshot", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		L"词典快照\0*.userdb.txt\0KCSS格式词典快照\0*.userdb.kct.snapshot\0全部文件\0*.*\0");
	if (IDOK == dlg.DoModal()) {
		char path[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, dlg.m_szFileName, -1, path, _countof(path), NULL, NULL);
		if (!api_->restore_user_dict(path)) {
			MessageBox(L"不知哪里出错了，未能完成操作。", L":-(", MB_OK | MB_ICONERROR);
		}
		else {
			MessageBox(L"完成了。", L":-)", MB_OK | MB_ICONINFORMATION);
		}
	}
	return 0;
}

LRESULT DictManagementDialog::OnExport(WORD, WORD code, HWND, BOOL&) {
	int sel = user_dict_list_.GetCurSel();
	if (sel < 0 || sel >= user_dict_list_.GetCount()) {
		MessageBox(L"请在左列选择要导出的词典名称。", L":-(", MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	WCHAR dict_name[MAX_PATH] = {0};
	user_dict_list_.GetText(sel, dict_name);
	std::wstring file_name(dict_name);
	file_name += L"_export.txt";
	CFileDialog dlg(FALSE, L"txt", file_name.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"文本文档\0*.txt\0全部文件\0*.*\0");
	if (IDOK == dlg.DoModal()) {
		char path[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, dlg.m_szFileName, -1, path, _countof(path), NULL, NULL);
		int result = api_->export_user_dict(to_string(dict_name, CP_UTF8).data(), path);
		if (result < 0) {
			MessageBox(L"不知哪里出错了，未能完成操作。", L":-(", MB_OK | MB_ICONERROR);
		}
		else if (_waccess(dlg.m_szFileName, 0) != 0) {
			MessageBox(L"咦，导出的文件找不着了。", L":-(", MB_OK | MB_ICONERROR);
		}
		else {
			std::wstring report{ std::format(L"导出了{}条记录。", result) };
			MessageBox(report.data(), L":-)", MB_OK | MB_ICONINFORMATION);
			std::wstring param{ std::format(LR"(/select, "{}")", dlg.m_szFileName) };
			ShellExecute(NULL, L"open", L"explorer.exe", param.data(), NULL, SW_SHOWNORMAL);
		}
	}
	return 0;
}

LRESULT DictManagementDialog::OnImport(WORD, WORD code, HWND, BOOL&) {
	int sel = user_dict_list_.GetCurSel();
	if (sel < 0 || sel >= user_dict_list_.GetCount()) {
		MessageBox(L"请在左列选择要导入的词典名称。", L":-(", MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	WCHAR dict_name[MAX_PATH] = {0};
	user_dict_list_.GetText(sel, dict_name);
	std::wstring file_name(dict_name);
	file_name += L"_export.txt";
	CFileDialog dlg(TRUE, L"txt", file_name.c_str(), OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, L"文本文档\0*.txt\0全部文件\0*.*\0");
	if (IDOK == dlg.DoModal()) {
		char path[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, dlg.m_szFileName, -1, path, _countof(path), NULL, NULL);
		int result = api_->import_user_dict(to_string(dict_name, CP_UTF8).data(), path);
		if (result < 0) {
			MessageBox(L"不知哪里出错了，未能完成操作。", L":-(", MB_OK | MB_ICONERROR);
		}
		else {
			std::wstring report{ std::format(L"导入了{}条记录。", result) };
			MessageBox(report.data(), L":-)", MB_OK | MB_ICONINFORMATION);
		}
	}
	return 0;
}

LRESULT DictManagementDialog::OnUserDictListSelChange(WORD, WORD, HWND, BOOL&) {
	int index = user_dict_list_.GetCurSel();
	BOOL enabled = index < 0 ? FALSE : TRUE;
	backup_.EnableWindow(enabled);
	export_.EnableWindow(enabled);
	import_.EnableWindow(enabled);
	return 0;
}

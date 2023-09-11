module;
#include "stdafx.h"
#include "resource.h"
#include "WeaselDeployer.h"
#include <dwmapi.h>
module UIStyleSettingsDialog;
import UIStyleSettings;
import Config;
import WeaselUtility;

#pragma comment(lib, "Dwmapi.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

UIStyleSettingsDialog::UIStyleSettingsDialog(UIStyleSettings* settings)
	: settings_(settings), loaded_(false)
{
}


UIStyleSettingsDialog::~UIStyleSettingsDialog()
{
	image_.Destroy();
	g_hwnd = nullptr;
}


void UIStyleSettingsDialog::Populate() {
	if (!settings_) return;
	std::string active(settings_->GetActiveColorScheme());
	int active_index = -1;
	settings_->GetPresetColorSchemes(&preset_);
	for (int i = 0; i < preset_.size(); ++i) {
		color_schemes_.AddString(to_wstring(preset_[i].name, CP_UTF8).data());
		if (preset_[i].color_scheme_id == active) {
			active_index = i;
		}
	}
	if (active_index >= 0) {
		color_schemes_.SetCurSel(active_index);
		Preview(active_index);
	}
	loaded_ = true;
}

LRESULT UIStyleSettingsDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	BOOL value{ TRUE };
	::DwmSetWindowAttribute(m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
	color_schemes_.Attach(GetDlgItem(IDC_COLOR_SCHEME));
	preview_.Attach(GetDlgItem(IDC_PREVIEW));
	select_font_.Attach(GetDlgItem(IDC_SELECT_FONT));
	select_font_.EnableWindow(FALSE);
	
	Populate();	
	
	CenterWindow();
	BringWindowToTop();
	g_hwnd = m_hWnd;
	return TRUE;
}

LRESULT UIStyleSettingsDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&) {
	CloseDialog(0);
	return 0;
}

LRESULT UIStyleSettingsDialog::OnOK(WORD, WORD code, HWND, BOOL&) {
	CloseDialog(0);
	return 0;
}

LRESULT UIStyleSettingsDialog::OnColorSchemeSelChange(WORD, WORD, HWND, BOOL&) {
	int index = color_schemes_.GetCurSel();
	if (index >= 0 && index < (int)preset_.size()) {
		settings_->SelectColorScheme(preset_[index].color_scheme_id);
		Preview(index);
	}
	return 0;
}

void UIStyleSettingsDialog::Preview(int index) {
	namespace fs = std::filesystem;
	if (index < 0 || index >= (int)preset_.size()) return;
	fs::path file_path(settings_->GetColorSchemePreview(preset_[index].color_scheme_id));
	if (!fs::exists(file_path)) return;
	image_.Destroy();
	image_.Load(file_path.wstring().data());
	if (!image_.IsNull()) {
		preview_.SetBitmap(image_);
	}
}

void UIStyleSettingsDialog::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

#pragma once

extern const GUID c_clsidTextService;

void enable_profile(BOOL fEnable, bool hant, const GUID& clsidTextServicee);
int register_text_service(const std::wstring& tsf_path, bool register_ime, bool is_wow64, bool hant, bool silent, const GUID& clsidTextService);
module;
#include "stdafx.h"
module WeaselTSF;

std::wstring subKey{ LR"(Software\Rime\Weasel)" };
std::wstring valueName{ L"GlobalCompartment" };

bool WeaselTSF::ReadConfiguration()
{
	HKEY hKey;
	auto ret = ::RegOpenKeyEx(HKEY_CURRENT_USER, subKey.data(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	DWORD dataSize = sizeof(DWORD);
	ret = ::RegQueryValueEx(hKey, valueName.data(), nullptr, nullptr, (LPBYTE)&m_globalCompartment, &dataSize);
	RegCloseKey(hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	return true;
}

bool WeaselTSF::WriteConfiguration()
{
	HKEY hKey;
	auto ret = ::RegOpenKeyEx(HKEY_CURRENT_USER, subKey.data(), 0, KEY_WRITE | KEY_WOW64_64KEY, &hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	ret = ::RegSetValueEx(hKey, valueName.data(), 0, REG_DWORD, (LPBYTE)&m_globalCompartment, sizeof(DWORD));
	RegCloseKey(hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	return true;
}
module;
#include "stdafx.h"
module WeaselTSF;

const std::wstring subKey{ LR"(Software\Rime\Weasel)" };
const std::wstring valueName{ L"GlobalCompartment" };
const std::wstring tsfHKEY{ L"TextService" };
const std::wstring fallbackPos{ L"FalbackPosition" };

bool WeaselTSF::ReadConfiguration(ConfigFlag flag)
{
	HKEY hKey;
	auto ret = ::RegOpenKeyEx(HKEY_CURRENT_USER, subKey.data(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	DWORD dataSize;
	switch (flag)
	{
	case ConfigFlag::GLOBAL_COMPARTMENT:
		dataSize = sizeof(DWORD);
		ret = ::RegQueryValueEx(hKey, valueName.data(), nullptr, nullptr, (LPBYTE)&m_globalCompartment, &dataSize);
		break;

	case ConfigFlag::FALLBACK_POSITION:
		dataSize = sizeof(RECT);
		ret = ::RegQueryValueEx(hKey, fallbackPos.data(), nullptr, nullptr, (LPBYTE)&m_rcFallback, &dataSize);
		break;
	}
	RegCloseKey(hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	return true;
}

bool WeaselTSF::WriteConfiguration(ConfigFlag flag)
{
	HKEY hKey;
	auto ret = ::RegOpenKeyEx(HKEY_CURRENT_USER, subKey.data(), 0, KEY_WRITE | KEY_WOW64_64KEY, &hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	switch (flag)
	{
	case ConfigFlag::GLOBAL_COMPARTMENT:
		ret = ::RegSetValueEx(hKey, valueName.data(), 0, REG_DWORD, (LPBYTE)&m_globalCompartment, sizeof(DWORD));
		break;

	case ConfigFlag::FALLBACK_POSITION:
		ret = ::RegSetValueEx(hKey, fallbackPos.data(), 0, REG_BINARY, (LPBYTE)&m_rcFallback, sizeof(RECT));
		break;
	}
	RegCloseKey(hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	return true;
}

bool WeaselTSF::StoreTextServiceHandle(HKL hkl)
{
	HKEY hKey;
	auto ret = ::RegOpenKeyEx(HKEY_CURRENT_USER, subKey.data(), 0, KEY_WRITE | KEY_WOW64_64KEY, &hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	ret = ::RegSetValueEx(hKey, tsfHKEY.data(), 0, REG_QWORD, (LPBYTE)&hkl, sizeof(HKL));
	RegCloseKey(hKey);

	if (ret != ERROR_SUCCESS)
		return false;

	return true;
}
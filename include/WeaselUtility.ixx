module;
#include <Windows.h>
export module WeaselUtility;
import <string>;

export
{
	// UTF-8 conversion
	inline const char* wcstoutf8(const WCHAR* wstr)
	{
		int buffer_len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		static char* buffer;
		if (buffer)
			delete[]buffer;
		buffer = new char[buffer_len + 1];
		memset(buffer, 0, buffer_len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buffer, buffer_len, NULL, NULL);
		return buffer;
	}

	inline const WCHAR* utf8towcs(const char* utf8_str)
	{
		int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
		static WCHAR* wbuffer;
		if (wbuffer)
			delete[]wbuffer;
		wbuffer = new WCHAR[nLen + 1];
		memset(wbuffer, 0, sizeof(WCHAR) * (nLen + 1));
		MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, wbuffer, nLen);
		return wbuffer;
	}

	inline int utf8towcslen(const char* utf8_str, int utf8_len)
	{
		return MultiByteToWideChar(CP_UTF8, 0, utf8_str, utf8_len, NULL, 0);
	}

	inline std::wstring getUsername() {
		DWORD len = 0;
		GetUserName(NULL, &len);

		if (len <= 0) {
			return L"";
		}

		wchar_t* username = new wchar_t[len + 1];

		GetUserName(username, &len);
		if (len <= 0) {
			delete[] username;
			return L"";
		}
		auto res = std::wstring(username);
		delete[] username;
		return res;
	}

	// data directories
	inline std::wstring WeaselUserDataPath()
	{
		WCHAR path[MAX_PATH] = { 0 };
		const WCHAR KEY[] = L"Software\\Rime\\Weasel";
		HKEY hKey;
		LSTATUS ret = RegOpenKey(HKEY_CURRENT_USER, KEY, &hKey);
		if (ret == ERROR_SUCCESS)
		{
			DWORD len = sizeof(path);
			DWORD type = 0;
			DWORD data = 0;
			ret = RegQueryValueEx(hKey, L"RimeUserDir", NULL, &type, (LPBYTE)path, &len);
			RegCloseKey(hKey);
			if (ret == ERROR_SUCCESS && type == REG_SZ && path[0])
			{
				return path;
			}
		}
		// default location
		ExpandEnvironmentStringsW(L"%AppData%\\Rime", path, _countof(path));
		return path;
	}

	inline std::wstring WeaselRootPath()
	{
		WCHAR path[MAX_PATH] = { 0 };
		const WCHAR KEY[] = L"Software\\Rime\\Weasel";
		HKEY hKey;
#ifdef _M_IX86
		PVOID OldValue = NULL;
		using Wow64DisableWow64FsRedirectionFunc = BOOL(__stdcall*)(PVOID*);
		using Wow64ReverWow64FsRedirectionFunc = BOOL(__stdcall*)(PVOID);
		Wow64DisableWow64FsRedirectionFunc Wow64DisableWow64FsRedirection{ (Wow64DisableWow64FsRedirectionFunc)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "Wow64DisableWow64FsRedirection") };
		Wow64ReverWow64FsRedirectionFunc Wow64RevertWow64FsRedirection{ (Wow64ReverWow64FsRedirectionFunc)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "Wow64RevertWow64FsRedirection") };
		if (Wow64DisableWow64FsRedirection)
		{
			Wow64DisableWow64FsRedirection(&OldValue);
		}
		LSTATUS ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey);
#else
		LSTATUS ret = RegOpenKey(HKEY_LOCAL_MACHINE, KEY, &hKey);
#endif // _M_IX86
		
		if (ret == ERROR_SUCCESS)
		{
			DWORD len = sizeof(path);
			DWORD type = 0;
			DWORD data = 0;
			ret = RegQueryValueEx(hKey, L"WeaselRoot", NULL, &type, (LPBYTE)path, &len);
			RegCloseKey(hKey);

#ifdef _M_IX86
			if (Wow64RevertWow64FsRedirection)
			{
				Wow64RevertWow64FsRedirection(OldValue);
			}
#endif // _M_IX86
			if (ret == ERROR_SUCCESS && type == REG_SZ && path[0])
			{
				return path;
			}
		}
		return path;
	}

	inline const char* weasel_shared_data_dir()
	{
		static char path[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, path, _countof(path));
		std::string str_path(path);
		size_t k = str_path.find_last_of("/\\");
		strcpy_s(path + k + 1, _countof(path) - (k + 1), "data");
		return path;
	}

	inline const char* weasel_user_data_dir()
	{
		static char path[MAX_PATH] = { 0 };
		// Windows wants multi-byte file paths in native encoding
		WideCharToMultiByte(CP_ACP, 0, WeaselUserDataPath().c_str(), -1, path, _countof(path) - 1, NULL, NULL);
		return path;
	}

	inline std::string to_string(const std::wstring& input, int code_page = CP_ACP)
	{
		auto len = WideCharToMultiByte(code_page, 0, input.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string temp;

		if (len > 0)
		{
			temp.resize(len);
			WideCharToMultiByte(code_page, 0, input.c_str(), -1, &temp[0], len, nullptr, nullptr);
			temp = temp.data();
		}

		return temp;
	}

	inline std::wstring to_wstring(const std::string& input, int code_page = CP_ACP)
	{
		auto len = MultiByteToWideChar(code_page, 0, input.c_str(), -1, nullptr, 0);
		std::wstring temp;

		if (len > 0)
		{
			temp.resize(len);
			MultiByteToWideChar(code_page, 0, input.c_str(), -1, &temp[0], len);
			temp = temp.data();
		}

		return temp;
	}

	// resource
	inline std::string GetCustomResource(const char* name, const char* type)
	{
		const HINSTANCE module = 0; // main executable
		HRSRC hRes = FindResourceA(module, name, type);
		if (hRes)
		{
			HGLOBAL hData = LoadResource(module, hRes);
			if (hData)
			{
				const char* data = (const char*)::LockResource(hData);
				size_t size = ::SizeofResource(module, hRes);

				if (data && size)
				{
					if (data[size - 1] == '\0') // null-terminated string
						size--;
					return std::string(data, size);
				}
			}
		}

		return std::string();
	}
}
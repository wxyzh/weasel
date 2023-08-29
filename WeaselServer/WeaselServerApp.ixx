module;
#include "stdafx.h"
#include "resource.h"
#include <winsparkle.h>
export module WeaselServerApp;
import <functional>;
import <memory>;
import <string>;
import <filesystem>;
import <format>;
import WeaselIPC;
import WeaselUI;
import RimeWithWeasel;
import WeaselUtility;
import WeaselTrayIcon;

namespace fs = std::filesystem;

export
{
	class WeaselServerApp {
	public:
		static void checked()
		{
			using MessageBoxTimeoutFunc = int (__stdcall*)(HWND hwnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds);
			auto hUser32 = LoadLibrary(L"user32.dll");
			if (hUser32)
			{
				MessageBoxTimeoutFunc MessageBoxTimeout{ (MessageBoxTimeoutFunc)::GetProcAddress(hUser32, "MessageBoxTimeoutW") };
				if (MessageBoxTimeout)
				{
					MessageBoxTimeout(nullptr, L"dummy!", L"", MB_OK, 0, 50);
					FreeLibrary(hUser32);
				}
			}
		}

		static bool execute(std::wstring_view cmd, std::wstring_view args)
		{
			return (int)ShellExecuteW(nullptr, NULL, cmd.data(), args.data(), NULL, SW_SHOWNORMAL) > 32;
		}

		static bool explore(std::wstring_view path)
		{
			return (int)ShellExecuteW(nullptr, L"open", L"explorer", path.data(), NULL, SW_SHOWNORMAL) > 32;
		}

		static bool open(std::wstring_view path)
		{
			return (int)ShellExecuteW(nullptr, L"open", path.data(), NULL, NULL, SW_SHOWNORMAL) > 32;
		}

		static bool check_update()
		{
			// when checked manually, show testing versions too
			std::string feed_url = GetCustomResource("ManualUpdateFeedURL", "APPCAST");
			if (!feed_url.empty())
			{
				win_sparkle_set_appcast_url(feed_url.c_str());
			}
			win_sparkle_check_update_with_ui();
			return true;
		}

		static std::wstring install_dir()
		{
			WCHAR exe_path[MAX_PATH] = { 0 };
			GetModuleFileNameW(GetModuleHandle(NULL), exe_path, _countof(exe_path));
			std::wstring dir(exe_path);
			size_t pos = dir.find_last_of(L"\\");
			dir.resize(pos);
			return dir;
		}

	public:
		WeaselServerApp();
		~WeaselServerApp();
		int Run();

	protected:
		void SetupMenuHandlers();

		weasel::Server m_server;
		weasel::UI m_ui;
		WeaselTrayIcon tray_icon;
		std::unique_ptr<RimeWithWeaselHandler> m_handler;
	};
}
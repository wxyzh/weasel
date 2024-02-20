module;
#include "stdafx.h"
#include <WeaselUI.h>
#include "resource.h"
#include <winsparkle.h>
export module WeaselServerApp;
import <dwrite_2.h>;
import <functional>;
import <memory>;
import <string>;
import <filesystem>;
import <format>;
import WeaselIPC;
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

		static bool view_log()
		{
			fs::path log_path{ weasel_log_dir() };
			auto id = GetCurrentProcessId();

			fs::recursive_directory_iterator begin{ log_path };
			fs::recursive_directory_iterator end{};
			std::vector<std::wstring> logs{};
			logs.reserve(5);
			for (auto iter{ begin }; iter != end; ++iter)
			{
				auto& entry{ *iter };
				if (fs::is_regular_file(entry) && stoul(entry.path().extension().string().substr(1)) == id)
				{
					std::wstring cmdLine{ std::format(LR"(notepad {})", entry.path().wstring()) };
					logs.emplace_back(std::move(cmdLine));
				}
			}

			for (auto item : logs)
			{
				STARTUPINFO si{ sizeof(si) };
				PROCESS_INFORMATION pi{};
				auto ret = CreateProcess(nullptr, item.data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);
				if (ret)
				{
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
				}
			}
			return true;
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
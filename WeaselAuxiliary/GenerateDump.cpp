module;
#include "pch.h"
#include "framework.h"
#include <Windows.h>
#include <DbgHelp.h>
#include <ShlObj_core.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Shcore.lib")
module GenerateDump;

using namespace std::chrono;
namespace fs = std::filesystem;

std::shared_ptr<CreateDump*> CreateDump::m_instance = std::make_shared<CreateDump*>();
std::wstring CreateDump::m_dumpFile = L"";

CreateDump::CreateDump()
{
}

CreateDump::~CreateDump()
{
}

CreateDump* CreateDump::Instance()
{
	if (*m_instance == nullptr)
	{
		m_instance = std::make_shared<CreateDump*>(new CreateDump);
		// std::wcout << L"*m_instance == nullptr\n";
	}

	return *m_instance;
}

long __stdcall CreateDump::UnhandleExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
	// std::wcout << L"From UnhandleExceptionFilter\n";
	HANDLE hFile = ::CreateFile(m_dumpFile.data(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION exInfo{};
		exInfo.ThreadId = ::GetCurrentThreadId();
		exInfo.ExceptionPointers = ExceptionInfo;
		exInfo.ClientPointers = FALSE;

		// write the dump
		auto bOK = ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, &exInfo, nullptr, nullptr);
		::CloseHandle(hFile);
		if (!bOK)
		{
			auto dw = ::GetLastError();
			// std::wcout << L"文件创建成功！\n";
			// 写dump文件出错处理，异常交给Windows处理
			return EXCEPTION_CONTINUE_SEARCH;
		}
		else // 在异常处结束
		{
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}
	else
	{
		// std::wcout << L"文件创建失败！\n";
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

void CreateDump::DeclarDumpFile(std::wstring_view dmpFileName)
{
	auto currentTime{ floor<seconds>(current_zone()->to_local(system_clock::now())) };
	m_dumpFile = std::format(L"{}{:%F %H-%M-%S}.dmp", dmpFileName, currentTime);
	
	auto ret = ::SetUnhandledExceptionFilter(UnhandleExceptionFilter);
	// std::wcout << std::format(L"dmpFileName = {}, ret = 0x{:X}\n", dmpFileName.data(), (size_t)ret);
}

void CatchUnhandledException()
{
	std::wstring desktop;
	desktop.reserve(MAX_PATH);

	::SHGetFolderPath(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, desktop.data());

	desktop = desktop.data();
	CreateDump::Instance()->DeclarDumpFile(fs::path(desktop).append(LR"(rime_dump_)").wstring().data());
}
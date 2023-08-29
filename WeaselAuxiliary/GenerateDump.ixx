module;
#include "pch.h"
#include "framework.h"
#include <Windows.h>
#include <DbgHelp.h>
export module GenerateDump;
import <memory>;
import <string>;
import <filesystem>;
import <format>;
import <chrono>;

export
{
	class CreateDump
	{
	public:
		CreateDump();
		~CreateDump();

		static CreateDump* Instance();
		static long __stdcall UnhandleExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo);
		// 声明Dump文件，异常时会自动生成。会自动加入.dmp文件名后缀
		void DeclarDumpFile(std::wstring_view dmpFileName = L"");

	private:
		static std::wstring m_dumpFile;
		static std::shared_ptr<CreateDump*> m_instance;
	};

	void CatchUnhandledException();
}
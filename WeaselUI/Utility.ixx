module;
#include <Windows.h>
export module Utility;
import <string>;

export
{
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
}
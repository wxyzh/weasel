module;
#include <windows.h>
#include <WeaselCommon.h>
export module ResponseParser;
import <map>;
import <memory>;
import <string>;


export namespace weasel
{
	class Deserializer;

	// 解析server回应文本
	struct ResponseParser
	{
		std::map<std::wstring, std::shared_ptr<Deserializer> > deserializers;

		std::wstring* p_commit;
		Context* p_context;
		Status* p_status;
		Config* p_config;
		UIStyle* p_style;

		ResponseParser(std::wstring* commit, Context* context = nullptr, Status* status = nullptr, Config* config = nullptr, UIStyle* style = nullptr);

		// 重载函数调用运算符, 以扮做ResponseHandler
		bool operator() (LPWSTR buffer, UINT length);

		// 处理一行回应文本
		void Feed(const std::wstring& line);
	};

}

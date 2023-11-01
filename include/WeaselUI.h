#pragma once
#include <WeaselCommon.h>
#include <memory>
#include <functional>

namespace weasel
{
	enum ClientCapabilities
	{
		INLINE_PREEDIT_CAPABLE = 1,
	};

	class DirectWriteResources;
	using PDWR = std::shared_ptr<DirectWriteResources>;

	//
	// 输入法界面接口类
	//
	class UI
	{
	public:
		UI();
		virtual ~UI();

		// 创建输入法界面
		bool Create(HWND parent);

		// 销毁界面
		void Destroy(bool full = false);
		// 退出应用后销毁界面及相应资源
		// void DestroyAll();
		
		// 界面显隐
		void Show();
		void Hide();
		void ShowWithTimeout(DWORD millisec);
		bool IsCountingDown() const;
		bool IsShown() const;
		
		// 重绘界面
		void Refresh();

		// 置输入焦点位置（光标跟随时移动候选窗）但不重绘
		void UpdateInputPosition(RECT const& rc);

		// 更新界面显示内容
		void Update(Context const& ctx, Status const& status);

		Context& ctx();
		Context& octx();
		Status& status();
		UIStyle& style();
		UIStyle& ostyle();
		PDWR pdwr();
		bool GetIsReposition();

		void SetCaretFollowing(const bool following);
		bool GetCaretFollowing() const;

		std::function<void(int* const, int* const, bool* const)>& uiCallback();
		void SetSelectCallback(std::function<void(int* const, int* const, bool* const)> const& func);

		std::function<void (const RECT&)>& SetRectCallback();
		void SetRectCallback(std::function<void (const RECT&)> func);

	private:
		struct Data;
		class UIImpl;

		std::unique_ptr<Data> m_data;		
		std::unique_ptr<UIImpl> pimpl_;
	};
}

#pragma once
#include <WeaselCommon.h>
#include <memory>
#include <functional>
#include <Windows.h>

namespace weasel
{
	template <typename T>
	void SafeRelease(T** ppT)
	{
		if (*ppT)
		{
			(*ppT)->Release();
			*ppT = nullptr;
		}
	}

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

		Context& ctx() { return ctx_; }
		Context& octx() { return octx_; }
		Status& status() { return status_; } 
		UIStyle& style() { return style_; }
		UIStyle& ostyle() { return ostyle_; }
		PDWR pdwr() { return pDWR_; }
		bool GetIsReposition();

		void SetCaretFollowing(const bool following);
		bool GetCaretFollowing() const { return _CaretFollowing; }

		std::function<void(int* const, int* const, bool* const)>& uiCallback() { return _UICallback; }
		void SetSelectCallback(std::function<void(int* const, int* const, bool* const)> const& func) { _UICallback = func; }

		std::function<void(const RECT&)>& SetRectCallback() { return _SetRectCallback; }
		void SetRectCallback(std::function<void(const RECT&)> func) { _SetRectCallback = func; }

	private:
		class UIImpl;
		std::unique_ptr<UIImpl> pimpl_;
		PDWR pDWR_;

		Context ctx_;		
		Context octx_;
		Status status_;
		UIStyle style_;
		UIStyle ostyle_;
		std::function<void(int* const, int* const, bool* const)> _UICallback;
		std::function<void(const RECT&)> _SetRectCallback;
		bool _CaretFollowing{ true };
	};
}

module;
#include <d2d1.h>
#include <dwrite_2.h>
#include <ShellScalingApi.h>
#include "VersionHelpers.hpp"
export module WeaselUI;
import WeaselCommon;
import <vector>;
import <regex>;
import <memory>;
import <string>;
import <span>;
import <functional>;

export namespace weasel
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

	class UIImpl;
	class DirectWriteResources;
	using PDWR = std::shared_ptr<DirectWriteResources>;

	//
	// 输入法界面接口类
	//
	class UI
	{
	public:
		UI() : pimpl_(0)
		{
		}

		virtual ~UI()
		{
			if (pimpl_)
				Destroy(true);
			if (pDWR_)
			{
				pDWR_ = nullptr;
			}
		}

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
		void Refresh(bool from_server);

		// 置输入焦点位置（光标跟随时移动候选窗）但不重绘
		void UpdateInputPosition(RECT const& rc);

		// 更新界面显示内容
		void Update(Context const& ctx, Status const& status, bool from_server = false);

		Context& ctx() { return ctx_; }
		Context& octx() { return octx_; }
		Status& status() { return status_; } 
		UIStyle& style() { return style_; }
		UIStyle& ostyle() { return ostyle_; }
		PDWR pdwr() { return pDWR_; }
		std::function<void(std::wstring_view, const size_t index)>& selectCallback() { return _SelectCallback; }
		void SetSelectCallback(std::function<void(std::wstring_view, const size_t index)> const& func) { _SelectCallback = func; }

	private:
		UIImpl* pimpl_;
		PDWR pDWR_;

		Context ctx_;
		Context octx_;
		Status status_;
		UIStyle style_;
		UIStyle ostyle_;
		std::function<void(std::wstring_view, const size_t index)> _SelectCallback;
	};

	class DirectWriteResources
	{
	public:
		DirectWriteResources(weasel::UIStyle& style, UINT dpi);
		~DirectWriteResources();

		HRESULT InitResources(std::wstring_view label_font_face, int lavel_font_point,
			std::wstring_view font_face, int font_point,
			std::wstring_view comment_font_face, int comment_font_point, bool vertical_text = false);
		HRESULT InitResources(UIStyle& style, UINT dpi);
		void SetDpi(UINT dpi);

	public:
		float dpiScaleX_, dpiScaleY_;
		ID2D1Factory* pD2d1Factory;
		IDWriteFactory2* pDWFactory;
		ID2D1DCRenderTarget* pRenderTarget;
		IDWriteTextFormat1* pPreeditTextFormat;
		IDWriteTextFormat1* pTextFormat;
		IDWriteTextFormat1* pLabelTextFormat;
		IDWriteTextFormat1* pCommentTextFormat;
		IDWriteTextLayout2* pTextLayout;
		ID2D1SolidColorBrush* m_pBrush;

	private:
		void _ParseFontFace(const std::wstring& fontFaceStr, DWRITE_FONT_WEIGHT& fontWeight, DWRITE_FONT_STYLE& fontStyle);
		void _SetFontFallback(IDWriteTextFormat1* pTextFormat, std::span<std::wstring> fontVec);

	private:		
		UIStyle& _style;		
	};
}

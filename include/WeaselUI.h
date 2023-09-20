#pragma once
#include <WeaselCommon.h>
#include <atlcomcli.h> 
#include <ShellScalingApi.h>
#include "VersionHelpers.hpp"
#include <d2d1.h>
#include <dwrite_2.h>
#include <functional>
#include <memory>
#include <wrl/client.h>
#include <span>
using namespace Microsoft::WRL;

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

	class UIImpl;
	class DirectWriteResources;
	template<typename T>
	using an = std::shared_ptr<T>;
	using PDWR = an<DirectWriteResources>;

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
		bool GetIsReposition();

		void SetCaretFollowing(const bool following);
		bool GetCaretFollowing() const { return _CaretFollowing; }

		std::function<void(int* const, int* const, bool* const)>& uiCallback() { return _UICallback; }
		void SetSelectCallback(std::function<void(int* const, int* const, bool* const)> const& func) { _UICallback = func; }

		std::function<void(const RECT&)>& SetRectCallback() { return _SetRectCallback; }
		void SetRectCallback(std::function<void(const RECT&)> func) { _SetRectCallback = func; }		

	private:
		UIImpl* pimpl_;
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

	class DirectWriteResources
	{
	public:
		DirectWriteResources(weasel::UIStyle& style, UINT dpi = 96);
		~DirectWriteResources();

		HRESULT InitResources(std::wstring_view label_font_face, int lavel_font_point,
			std::wstring_view font_face, int font_point,
			std::wstring_view comment_font_face, int comment_font_point, bool vertical_text = false);
		HRESULT InitResources(UIStyle& style, const UINT dpi);

		HRESULT CreateTextLayout(std::wstring_view text, const int nCount, IDWriteTextFormat1* const txtFormat, const float width, const float height);
		HRESULT CreateBrush(const D2D1_COLOR_F& color);

		HRESULT GetLayoutOverhangMetrics(DWRITE_OVERHANG_METRICS* overhangMetrics);
		HRESULT GetLayoutMetrics(DWRITE_TEXT_METRICS* metrics);
		HRESULT SetLayoutReadingDirection(const DWRITE_READING_DIRECTION& direct);
		HRESULT SetLayoutFlowDirection(const DWRITE_FLOW_DIRECTION& direct);
		
		void DrawRect(D2D1_RECT_F* const rect, const float strokeWidth = 1.0f, ID2D1StrokeStyle* const sstyle = nullptr);
		void DrawTextLayoutAt(const D2D1_POINT_2F& point);

		void ResetLayout() { pTextLayout.Reset(); }
		void SetBrushColor(const D2D1_COLOR_F& color) { m_pBrush->SetColor(color); }
		void SetDpi(UINT dpi);

	public:
		float dpiScaleX_, dpiScaleY_;
		ComPtr<ID2D1Factory> pD2d1Factory;
		ComPtr<IDWriteFactory2> pDWFactory;
		ComPtr<ID2D1DCRenderTarget> pRenderTarget;
		ComPtr<IDWriteTextFormat1> pPreeditTextFormat;
		ComPtr<IDWriteTextFormat1> pTextFormat;
		ComPtr<IDWriteTextFormat1> pLabelTextFormat;
		ComPtr<IDWriteTextFormat1> pCommentTextFormat;
		ComPtr<IDWriteTextLayout2> pTextLayout;
		ComPtr<ID2D1SolidColorBrush> m_pBrush;

	private:
		void _ParseFontFace(const std::wstring& fontFaceStr, DWRITE_FONT_WEIGHT& fontWeight, DWRITE_FONT_STYLE& fontStyle);
		void _SetFontFallback(ComPtr<IDWriteTextFormat1> pTextFormat, std::span<std::wstring> fontVec);

	private:		
		UIStyle& _style;		
	};
}

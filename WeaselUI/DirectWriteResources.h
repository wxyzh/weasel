#pragma once
#include <WeaselCommon.h>

using namespace Microsoft::WRL;

namespace weasel
{
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
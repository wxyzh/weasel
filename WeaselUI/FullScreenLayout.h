#pragma once

#include "StandardLayout.h"

namespace weasel
{
	class FullScreenLayout: public StandardLayout
	{
	public:
		FullScreenLayout(const UIStyle &style, const Context &context, const Status &status, const CRect& inputPos, std::shared_ptr<Layout> layout)
			: StandardLayout(style, context, status), mr_inputPos(inputPos), m_layout(layout) { }
		virtual ~FullScreenLayout(){ }

		virtual void DoLayout(CDCHandle dc, PDWR pDWR = nullptr);

	private:
		bool AdjustFontPoint(CDCHandle dc, const CRect& workArea, int& step, PDWR pDWR = nullptr);

		const CRect& mr_inputPos;
		std::shared_ptr<Layout> m_layout;
	};
}
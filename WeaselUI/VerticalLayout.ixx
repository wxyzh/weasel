module;
#include "stdafx.h"
export module VerticalLayout;
import StandardLayout;

export namespace weasel
{
	class VerticalLayout: public StandardLayout
	{
	public:
		VerticalLayout(const UIStyle &style, const Context &context, const Status &status)
			: StandardLayout(style, context, status) {}
		virtual void DoLayout(CDCHandle dc, PDWR pDWR = nullptr);
	};
};

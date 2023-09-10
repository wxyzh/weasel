module;
#include "stdafx.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
export module GdiplusBlur;

export
{
	namespace weasel
	{
		void DoGaussianBlur(Gdiplus::Bitmap* img, float radiusX, float radiusY);
	}
}
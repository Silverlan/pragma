#ifndef __UTIL_RGBCSV_HPP__
#define __UTIL_RGBCSV_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/color.h>

namespace util
{
	struct DLLNETWORK HSV
	{
		double h;
		double s;
		double v;
	};
	DLLNETWORK HSV rgb_to_hsv(const Color &rgb);
	DLLNETWORK Color hsv_to_rgb(const HSV &hsv);
	DLLNETWORK HSV lerp_hsv(const HSV &src,const HSV &dst,float t);
};

#endif

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_RGBCSV_HPP__
#define __UTIL_RGBCSV_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/color.h>

namespace util {
	struct DLLNETWORK HSV {
		HSV(double h, double s, double v) : h {h}, s {s}, v {v} {}
		HSV() = default;

		bool operator==(const util::HSV &b) const
		{
			constexpr auto EPSILON = 0.01f;
			return umath::abs(h - b.h) <= EPSILON && umath::abs(s - b.s) <= EPSILON && umath::abs(v - b.v) <= EPSILON;
		}
		bool operator!=(const util::HSV &b) const { return operator==(b); }

		double h = 0.0;
		double s = 0.0;
		double v = 1.0;
	};
	DLLNETWORK HSV rgb_to_hsv(const Color &rgb);
	DLLNETWORK Color hsv_to_rgb(const HSV &hsv);
	DLLNETWORK HSV lerp_hsv(const HSV &src, const HSV &dst, float t);
};

#endif

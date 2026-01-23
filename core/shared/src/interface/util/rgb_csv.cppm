// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.rgb_csv;

export import pragma.math;

export namespace pragma::util {
	struct DLLNETWORK HSV {
		HSV(double h, double s, double v) : h {h}, s {s}, v {v} {}
		HSV() = default;

		bool operator==(const HSV &b) const
		{
			constexpr auto EPSILON = 0.01f;
			return math::abs(h - b.h) <= EPSILON && math::abs(s - b.s) <= EPSILON && math::abs(v - b.v) <= EPSILON;
		}
		bool operator!=(const HSV &b) const { return operator==(b); }

		double h = 0.0;
		double s = 0.0;
		double v = 1.0;
	};
	DLLNETWORK HSV rgb_to_hsv(const Color &rgb);
	DLLNETWORK Color hsv_to_rgb(const HSV &hsv);
	DLLNETWORK HSV lerp_hsv(const HSV &src, const HSV &dst, float t);
};

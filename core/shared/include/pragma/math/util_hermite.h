// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_HERMITE_H__
#define __UTIL_HERMITE_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

namespace util {
	DLLNETWORK Vector3 calc_hermite_spline_position(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, float s, float curvature = 1.f);
	DLLNETWORK void calc_hermite_spline(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, uint32_t segmentCount, std::vector<Vector3> &curvePoints, float curvature = 1.f);
};

#endif

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.hermite;

export import pragma.math;

export namespace pragma::math {
	DLLNETWORK Vector3 calc_hermite_spline_position(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, float s, float curvature = 1.f);
	DLLNETWORK void calc_hermite_spline(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, uint32_t segmentCount, std::vector<Vector3> &curvePoints, float curvature = 1.f);
};

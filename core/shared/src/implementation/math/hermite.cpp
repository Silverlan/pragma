// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math.hermite;

static Vector4 get_hermite_transform(float s)
{
	auto s2 = s * s;
	auto s3 = s2 * s;
	return Vector4 {2.f * s3 - 3.f * s2 + 1.f, -2.f * s3 + 3.f * s2, s3 - 2.f * s2 + s, s3 - s2};
}

static void emit_curve_segment(float s, float sNext, const Vector3 &p1, const Vector3 &p2, const Vector3 &t1, const Vector3 &t2, Vector3 &r1, Vector3 &r2)
{
	auto h1 = get_hermite_transform(s);
	auto h2 = get_hermite_transform(sNext);

	r1 = h1.x * p1 + h1.y * p2 + h1.z * t1 + h1.w * t2;
	r2 = h2.x * p1 + h2.y * p2 + h2.z * t1 + h2.w * t2;
}

Vector3 pragma::math::calc_hermite_spline_position(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, float s, float curvature)
{
	auto ct1 = curvature * (p2 - p0);
	auto ct2 = curvature * (p3 - p1);

	auto h = get_hermite_transform(s);
	return h.x * p1 + h.y * p2 + h.z * ct1 + h.w * ct2;
}

void pragma::math::calc_hermite_spline(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, uint32_t segmentCount, std::vector<Vector3> &curvePoints, float curvature)
{
	auto ct1 = curvature * (p2 - p0);
	auto ct2 = curvature * (p3 - p1);

	curvePoints.reserve(curvePoints.size() + segmentCount * 2);
	auto inc = 1.f / static_cast<float>(segmentCount);
	Vector3 r1, r2;
	for(auto s = 0.f; s < 1.f; s += inc) {
		emit_curve_segment(s, s + inc, p1, p2, ct1, ct2, r1, r2);
		curvePoints.push_back(r1);
		curvePoints.push_back(r2);
	}
}

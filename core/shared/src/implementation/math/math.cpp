// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math;

Vector3 pragma::math::angular_velocity_to_linear(const Vector3 &refPos, const Vector3 &angVel, const Vector3 &tgtPos) { return uvec::cross(angVel, tgtPos - refPos); }

static double sqrt_3d(double d)
{
	if(d == 0.0)
		return 0.0;
	else if(d < 0.0)
		return -exp(log(-d) / 3.0);
	else
		return exp(log(d) / 3.0);
}

static uint32_t find_cubic_equation_roots(double c0, double c1, double c2, double c3, std::array<float, 3> &roots)
{
	double a, b, c, p, q, d, t, phi;
	uint32_t numResults = 0;
	constexpr float epsilon = -1.0e-10;
	if(c3 != 0.0) {
		a = c2 / c3;
		b = c1 / c3;
		c = c0 / c3;
		a = a / 3;

		p = b / 3 - a * a;
		q = (2 * a * a * a - a * b + c) / 2;
		d = q * q + p * p * p;

		if(d > 0.0) {
			t = sqrt(d);
			roots[0] = static_cast<float>(sqrt_3d(-q + t) + sqrt_3d(-q - t) - a);

			if((roots[0] >= epsilon) && (roots[0] <= 1.000001f))
				return 1;
			return 0;
		}

		if(d == 0.0) {
			t = sqrt_3d(-q);
			roots[0] = static_cast<float>(2 * t - a);

			if((roots[0] >= epsilon) && (roots[0] <= 1.000001f))
				numResults++;
			roots[numResults] = static_cast<float>(-t - a);

			if((roots[numResults] >= epsilon) && (roots[numResults] <= 1.000001f))
				return numResults + 1;
			return numResults;
		}

		phi = acos(-q / sqrt(-(p * p * p)));
		t = sqrt(-p);
		p = cos(phi / 3);
		q = sqrt(3 - 3 * p * p);
		roots[0] = static_cast<float>(2 * t * p - a);

		if((roots[0] >= epsilon) && (roots[0] <= 1.000001f))
			numResults++;
		roots[numResults] = static_cast<float>(-t * (p + q) - a);

		if((roots[numResults] >= epsilon) && (roots[numResults] <= 1.000001f))
			numResults++;
		roots[numResults] = static_cast<float>(-t * (p - q) - a);

		if((roots[numResults] >= epsilon) && (roots[numResults] <= 1.000001f))
			return numResults + 1;
		return numResults;
	}
	a = c2;
	b = c1;
	c = c0;

	if(a != 0.0) {
		p = b * b - 4 * a * c;

		if(p > 0) {
			p = sqrt(p);
			roots[0] = static_cast<float>((-b - p) / (2 * a));

			if((roots[0] >= epsilon) && (roots[0] <= 1.000001f))
				numResults++;
			roots[numResults] = static_cast<float>((-b + p) / (2 * a));

			if((roots[numResults] >= epsilon) && (roots[numResults] <= 1.000001f))
				return numResults + 1;
			return numResults;
		}

		if(p == 0) {
			roots[0] = static_cast<float>(-b / (2 * a));
			if((roots[0] >= epsilon) && (roots[0] <= 1.000001f))
				return 1;
		}
		return 0;
	}

	if(b != 0.0) {
		roots[0] = static_cast<float>(-c / b);

		if((roots[0] >= epsilon) && (roots[0] <= 1.000001f))
			return 1;
		return 0;
	}

	if(c == 0.0) {
		roots[0] = 0.0;
		return 1;
	}
	return 0;
}

uint32_t pragma::math::find_bezier_roots(float x, float v0, float v1, float v2, float v3, std::array<float, 3> &roots) { return find_cubic_equation_roots(v0 - x, 3.f * (v1 - v0), 3.f * (v0 - 2.f * v1 + v2), v3 - v0 + 3.f * (v1 - v2), roots); }

float pragma::math::calc_bezier_point(float f1, float f2, float f3, float f4, float t)
{
	auto c0 = f1;
	auto c1 = 3.f * (f2 - f1);
	auto c2 = 3.f * (f1 - 2.f * f2 + f3);
	auto c3 = f4 - f1 + 3.f * (f2 - f3);
	return c0 + t * c1 + pow2(t) * c2 + pow3(t) * c3;
}

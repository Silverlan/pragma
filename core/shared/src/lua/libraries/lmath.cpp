/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/libraries/lmath.h"
#include <pragma/engine.h>
#include <mathutil/umath.h>
#include "luasystem.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/math/util_hermite.h"
#include "pragma/util/util_ballistic.h"
#include "pragma/math/util_easing.hpp"
#include "pragma/model/modelmesh.h"
#include <mathutil/umath_equation_solver.h>
#include <mathutil/umath_frustum.hpp>
#include <mathutil/perlin_noise.hpp>

extern DLLNETWORK Engine *engine;
double Lua::math::perlin_noise(const Vector3 &v)
{
	umath::PerlinNoise p;
	return p.GetNoise(const_cast<Vector3 &>(v));
}
double Lua::math::perlin_noise(const Vector3 &v, uint32_t seed)
{
	umath::PerlinNoise p(seed);
	return p.GetNoise(const_cast<Vector3 &>(v));
}

float Lua::math::lerp(float a, float b, float f) { return a + f * (b - a); }

static float round_by_multiple(float f, float multiple) { return (multiple != 0.f) ? (::umath::round(f / multiple) * multiple) : f; }

float Lua::math::round(float f, float multiple) { return round_by_multiple(f, multiple); }

luabind::tableT<Vector3> Lua::math::calc_hermite_spline(lua_State *l, const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, uint32_t segmentCount, float curvature)
{
	std::vector<Vector3> curvePoints {};
	util::calc_hermite_spline(p0, p1, p2, p3, static_cast<uint32_t>(segmentCount), curvePoints, curvature);
	return Lua::vector_to_table(l, curvePoints);
}

Vector3 Lua::math::calc_hermite_spline_position(lua_State *l, const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, float s, float curvature) { return util::calc_hermite_spline_position(p0, p1, p2, p3, static_cast<float>(s), curvature); }

bool Lua::math::is_in_range(double v, double min, double max) { return v >= min && v <= max; }

template<int32_t nc, int32_t ns>
int32_t solve_equation(lua_State *l, const std::function<int32_t(const std::array<double, nc> &, std::array<double, ns> &)> &fSolve)
{
	int32_t idx = 1;
	std::array<double, nc> c;
	for(auto i = decltype(c.size()) {0}; i < c.size(); ++i)
		c[i] = Lua::CheckNumber(l, idx++);

	std::array<double, ns> s;
	auto n = fSolve(c, s);
	Lua::PushInt(l, n);
	for(auto i = decltype(n) {0}; i < n; ++i)
		Lua::PushNumber(l, s[i]);

	return n + 1;
}

int Lua::math::solve_quadric(lua_State *l) { return solve_equation<3, 2>(l, &umath::solve_quadric); }
int Lua::math::solve_cubic(lua_State *l) { return solve_equation<4, 3>(l, &umath::solve_cubic); }
int Lua::math::solve_quartic(lua_State *l) { return solve_equation<5, 4>(l, &umath::solve_quartic); }

luabind::optional<Vector3> Lua::math::calc_ballistic_velocity(lua_State *l, const Vector3 &start, const Vector3 &end, float angle, float gravity)
{
	Vector3 vel;
	auto r = umath::calc_ballistic_velocity(start, end, angle, gravity, vel);
	if(r == false)
		return nil;
	return object {l, vel};
}
luabind::tableT<Vector3> Lua::math::solve_ballistic_arc(lua_State *l, const Vector3 &projPos, double projSpeed, const Vector3 &tgt, const Vector3 &targetVel, double gravity)
{
	std::array<Vector3, 2> s;
	auto r = umath::solve_ballistic_arc(projPos, projSpeed, tgt, targetVel, gravity, s);
	luabind::tableT<Vector3> t = luabind::newtable(l);
	for(auto i = decltype(r) {0}; i < r; ++i)
		t[i + 1] = s[i];
	return t;
}
luabind::tableT<Vector3> Lua::math::solve_ballistic_arc(lua_State *l, const Vector3 &projPos, double projSpeed, const Vector3 &tgt, double gravity)
{
	std::array<Vector3, 2> s;
	auto r = umath::solve_ballistic_arc(projPos, projSpeed, tgt, gravity, s);
	luabind::tableT<Vector3> t = luabind::newtable(l);
	for(auto i = decltype(r) {0}; i < r; ++i)
		t[i + 1] = s[i];
	return t;
}
luabind::optional<luabind::mult<Vector3, double, Vector3>> Lua::math::solve_ballistic_arc_lateral(lua_State *l, const Vector3 &projPos, double lateralSpeed, const Vector3 &tgt, const Vector3 &targetVel, double maxHeightOffset)
{
	Vector3 fireVelocity;
	auto gravity = 0.f;
	Vector3 impactPoint;
	auto r = umath::solve_ballistic_arc_lateral(projPos, lateralSpeed, tgt, targetVel, maxHeightOffset, fireVelocity, gravity, impactPoint);
	if(!r)
		return nil;
	return luabind::mult<Vector3, double, Vector3> {l, fireVelocity, gravity, impactPoint};
}

luabind::optional<luabind::mult<Vector3, double>> Lua::math::solve_ballistic_arc_lateral(lua_State *l, const Vector3 &projPos, double lateralSpeed, const Vector3 &tgt, double maxHeight)
{
	Vector3 fireVelocity;
	auto gravity = 0.f;
	Vector3 impactPoint;
	auto r = umath::solve_ballistic_arc_lateral(projPos, lateralSpeed, tgt, maxHeight, fireVelocity, gravity);
	if(!r)
		return nil;
	return luabind::mult<Vector3, double> {l, fireVelocity, gravity};
}
int Lua::math::abs_max(lua_State *l)
{
	auto v = 0.f;
	int32_t argIdx = 1;
	while(Lua::IsSet(l, argIdx)) {
		auto n = Lua::CheckNumber(l, argIdx++);
		v = umath::abs_max(static_cast<float>(v), static_cast<float>(n));
	}
	Lua::PushNumber(l, v);
	return 1;
}
float Lua::math::ease_in(float t, umath::EaseType type) { return umath::ease_in(t, type); }
float Lua::math::ease_out(float t, umath::EaseType type) { return umath::ease_out(t, type); }
float Lua::math::ease_in_out(float t, umath::EaseType type) { return umath::ease_in_out(t, type); }

double Lua::math::calc_horizontal_fov(double focalLengthInMM, double width, double height) { return umath::rad_to_deg(::umath::calc_horizontal_fov(focalLengthInMM, width, height)); }
double Lua::math::calc_vertical_fov(double focalLengthInMM, double width, double height) { return umath::rad_to_deg(::umath::calc_vertical_fov(focalLengthInMM, width, height)); }
double Lua::math::calc_diagonal_fov(double focalLengthInMM, double width, double height) { return umath::rad_to_deg(::umath::calc_diagonal_fov(focalLengthInMM, width, height)); }

double Lua::math::horizontal_fov_to_vertical_fov(float fovDeg, float widthOrAspectRatio, float height) { return umath::rad_to_deg(::umath::horizontal_fov_to_vertical_fov(fovDeg, widthOrAspectRatio, height)); }
int Lua::math::vertical_fov_to_horizontal_fov(float fovDeg, float widthOrAspectRatio, float height) { return umath::rad_to_deg(::umath::vertical_fov_to_horizontal_fov(fovDeg, widthOrAspectRatio, height)); }
double Lua::math::diagonal_fov_to_vertical_fov(float diagonalFov, float aspectRatio) { return umath::rad_to_deg(::umath::diagonal_fov_to_vertical_fov(umath::deg_to_rad(diagonalFov), aspectRatio)); }

luabind::mult<float, float> Lua::math::get_frustum_plane_size(lua_State *l, float fovRad, float aspectRatio, float z)
{
	float w, h;
	::umath::frustum::get_plane_size(fovRad, z, aspectRatio, w, h);
	return {l, w, h};
}
luabind::mult<luabind::tableT<Vector3>, float, float> Lua::math::get_frustum_plane_boundaries(lua_State *l, const Vector3 &pos, const Vector3 &forward, const Vector3 &up, float fovRad, float aspectRatio, float z)
{
	float w, h;
	auto boundaries = ::umath::frustum::get_plane_boundaries(pos, forward, up, fovRad, z, aspectRatio, &w, &h);
	auto t = luabind::newtable(l);
	for(auto i = decltype(boundaries.size()) {0u}; i < boundaries.size(); ++i)
		t[i + 1] = boundaries[i];
	return {l, t, w, h};
}
Vector3 Lua::math::get_frustum_plane_point(lua_State *l, const Vector3 &pos, const Vector3 &forward, const Vector3 &right, const Vector3 &up, float fovRad, float aspectRatio, const ::Vector2 &uv, float z)
{
	return ::umath::frustum::get_plane_point(pos, forward, right, up, fovRad, z, aspectRatio, uv);
}

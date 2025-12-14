// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.math;

double Lua::math::perlin_noise(const Vector3 &v)
{
	pragma::math::PerlinNoise p;
	return p.GetNoise(const_cast<Vector3 &>(v));
}
double Lua::math::perlin_noise(const Vector3 &v, uint32_t seed)
{
	pragma::math::PerlinNoise p(seed);
	return p.GetNoise(const_cast<Vector3 &>(v));
}

float Lua::math::lerp(float a, float b, float f) { return a + f * (b - a); }

static float round_by_multiple(float f, float multiple) { return (multiple != 0.f) ? (pragma::math::round(f / multiple) * multiple) : f; }

float Lua::math::round(float f, float multiple) { return round_by_multiple(f, multiple); }

luabind::tableT<Vector3> Lua::math::calc_hermite_spline(lua::State *l, const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, uint32_t segmentCount, float curvature)
{
	std::vector<Vector3> curvePoints {};
	pragma::math::calc_hermite_spline(p0, p1, p2, p3, static_cast<uint32_t>(segmentCount), curvePoints, curvature);
	return vector_to_table(l, curvePoints);
}

Vector3 Lua::math::calc_hermite_spline_position(lua::State *l, const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, float s, float curvature) { return pragma::math::calc_hermite_spline_position(p0, p1, p2, p3, static_cast<float>(s), curvature); }

bool Lua::math::is_in_range(double v, double min, double max) { return v >= min && v <= max; }

template<int32_t nc, int32_t ns>
int32_t solve_equation(lua::State *l, const std::function<int32_t(const std::array<double, nc> &, std::array<double, ns> &)> &fSolve)
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

int Lua::math::solve_quadric(lua::State *l) { return solve_equation<3, 2>(l, &pragma::math::solve_quadric); }
int Lua::math::solve_cubic(lua::State *l) { return solve_equation<4, 3>(l, &pragma::math::solve_cubic); }
int Lua::math::solve_quartic(lua::State *l) { return solve_equation<5, 4>(l, &pragma::math::solve_quartic); }

luabind::optional<Vector3> Lua::math::calc_ballistic_velocity(lua::State *l, const Vector3 &start, const Vector3 &end, float angle, float gravity)
{
	Vector3 vel;
	auto r = pragma::math::calc_ballistic_velocity(start, end, angle, gravity, vel);
	if(r == false)
		return nil;
	return object {l, vel};
}
luabind::tableT<Vector3> Lua::math::solve_ballistic_arc(lua::State *l, const Vector3 &projPos, double projSpeed, const Vector3 &tgt, const Vector3 &targetVel, double gravity)
{
	std::array<Vector3, 2> s;
	auto r = pragma::math::solve_ballistic_arc(projPos, projSpeed, tgt, targetVel, gravity, s);
	luabind::tableT<Vector3> t = luabind::newtable(l);
	for(auto i = decltype(r) {0}; i < r; ++i)
		t[i + 1] = s[i];
	return t;
}
luabind::tableT<Vector3> Lua::math::solve_ballistic_arc(lua::State *l, const Vector3 &projPos, double projSpeed, const Vector3 &tgt, double gravity)
{
	std::array<Vector3, 2> s;
	auto r = pragma::math::solve_ballistic_arc(projPos, projSpeed, tgt, gravity, s);
	luabind::tableT<Vector3> t = luabind::newtable(l);
	for(auto i = decltype(r) {0}; i < r; ++i)
		t[i + 1] = s[i];
	return t;
}
luabind::optional<luabind::mult<Vector3, double, Vector3>> Lua::math::solve_ballistic_arc_lateral(lua::State *l, const Vector3 &projPos, double lateralSpeed, const Vector3 &tgt, const Vector3 &targetVel, double maxHeightOffset)
{
	Vector3 fireVelocity;
	auto gravity = 0.f;
	Vector3 impactPoint;
	auto r = pragma::math::solve_ballistic_arc_lateral(projPos, lateralSpeed, tgt, targetVel, maxHeightOffset, fireVelocity, gravity, impactPoint);
	if(!r)
		return nil;
	return luabind::mult<Vector3, double, Vector3> {l, fireVelocity, gravity, impactPoint};
}

luabind::optional<luabind::mult<Vector3, double>> Lua::math::solve_ballistic_arc_lateral(lua::State *l, const Vector3 &projPos, double lateralSpeed, const Vector3 &tgt, double maxHeight)
{
	Vector3 fireVelocity;
	auto gravity = 0.f;
	Vector3 impactPoint;
	auto r = pragma::math::solve_ballistic_arc_lateral(projPos, lateralSpeed, tgt, maxHeight, fireVelocity, gravity);
	if(!r)
		return nil;
	return luabind::mult<Vector3, double> {l, fireVelocity, gravity};
}
int Lua::math::abs_max(lua::State *l)
{
	auto v = 0.f;
	int32_t argIdx = 1;
	while(IsSet(l, argIdx)) {
		auto n = CheckNumber(l, argIdx++);
		v = pragma::math::abs_max(static_cast<float>(v), static_cast<float>(n));
	}
	PushNumber(l, v);
	return 1;
}
float Lua::math::ease_in(float t, pragma::math::EaseType type) { return pragma::math::ease_in(t, type); }
float Lua::math::ease_out(float t, pragma::math::EaseType type) { return pragma::math::ease_out(t, type); }
float Lua::math::ease_in_out(float t, pragma::math::EaseType type) { return pragma::math::ease_in_out(t, type); }

double Lua::math::calc_horizontal_fov(double focalLengthInMM, double width, double height) { return pragma::math::rad_to_deg(pragma::math::calc_horizontal_fov(focalLengthInMM, width, height)); }
double Lua::math::calc_vertical_fov(double focalLengthInMM, double width, double height) { return pragma::math::rad_to_deg(pragma::math::calc_vertical_fov(focalLengthInMM, width, height)); }
double Lua::math::calc_diagonal_fov(double focalLengthInMM, double width, double height) { return pragma::math::rad_to_deg(pragma::math::calc_diagonal_fov(focalLengthInMM, width, height)); }

double Lua::math::horizontal_fov_to_vertical_fov(float fovDeg, float widthOrAspectRatio, float height) { return pragma::math::rad_to_deg(pragma::math::horizontal_fov_to_vertical_fov(fovDeg, widthOrAspectRatio, height)); }
int Lua::math::vertical_fov_to_horizontal_fov(float fovDeg, float widthOrAspectRatio, float height) { return pragma::math::rad_to_deg(pragma::math::vertical_fov_to_horizontal_fov(fovDeg, widthOrAspectRatio, height)); }
double Lua::math::diagonal_fov_to_vertical_fov(float diagonalFov, float aspectRatio) { return pragma::math::rad_to_deg(pragma::math::diagonal_fov_to_vertical_fov(pragma::math::deg_to_rad(diagonalFov), aspectRatio)); }

luabind::mult<float, float> Lua::math::get_frustum_plane_size(lua::State *l, float fovRad, float aspectRatio, float z)
{
	float w, h;
	pragma::math::frustum::get_plane_size(fovRad, z, aspectRatio, w, h);
	return {l, w, h};
}
luabind::mult<luabind::tableT<Vector3>, float, float> Lua::math::get_frustum_plane_boundaries(lua::State *l, const Vector3 &pos, const Vector3 &forward, const Vector3 &up, float fovRad, float aspectRatio, float z)
{
	float w, h;
	auto boundaries = pragma::math::frustum::get_plane_boundaries(pos, forward, up, fovRad, z, aspectRatio, &w, &h);
	auto t = luabind::newtable(l);
	for(auto i = decltype(boundaries.size()) {0u}; i < boundaries.size(); ++i)
		t[i + 1] = boundaries[i];
	return {l, t, w, h};
}
Vector3 Lua::math::get_frustum_plane_point(lua::State *l, const Vector3 &pos, const Vector3 &forward, const Vector3 &right, const Vector3 &up, float fovRad, float aspectRatio, const ::Vector2 &uv, float z)
{
	return pragma::math::frustum::get_plane_point(pos, forward, right, up, fovRad, z, aspectRatio, uv);
}

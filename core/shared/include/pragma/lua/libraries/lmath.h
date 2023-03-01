/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LMATH_H__
#define __LMATH_H__

#include "pragma/networkdefinitions.h"
#include "pragma/math/util_easing.hpp"
#include <pragma/lua/luaapi.h>

namespace umath {
	enum class EaseType : uint32_t;
};
namespace Lua {
	namespace math {
		DLLNETWORK double perlin_noise(const Vector3 &v);
		DLLNETWORK double perlin_noise(const Vector3 &v, uint32_t seed);
		DLLNETWORK float lerp(float a, float b, float f);
		DLLNETWORK float round(float f, float multiple);
		DLLNETWORK luabind::tableT<Vector3> calc_hermite_spline(lua_State *l, const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, uint32_t segmentCount, float curvature = 1.f);
		DLLNETWORK Vector3 calc_hermite_spline_position(lua_State *l, const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, float s, float curvature = 1.f);
		DLLNETWORK bool is_in_range(double v, double min, double max);

		DLLNETWORK int solve_quadric(lua_State *l);
		DLLNETWORK int solve_cubic(lua_State *l);
		DLLNETWORK int solve_quartic(lua_State *l);
		DLLNETWORK luabind::optional<Vector3> calc_ballistic_velocity(lua_State *l, const Vector3 &start, const Vector3 &end, float angle, float gravity);

		DLLNETWORK luabind::tableT<Vector3> solve_ballistic_arc(lua_State *l, const Vector3 &projPos, double projSpeed, const Vector3 &tgt, const Vector3 &targetVel, double gravity);
		DLLNETWORK luabind::tableT<Vector3> solve_ballistic_arc(lua_State *l, const Vector3 &projPos, double projSpeed, const Vector3 &tgt, double gravity);

		DLLNETWORK luabind::optional<luabind::mult<Vector3, double, Vector3>> solve_ballistic_arc_lateral(lua_State *l, const Vector3 &projPos, double lateralSpeed, const Vector3 &tgt, const Vector3 &targetVel, double maxHeightOffset);
		DLLNETWORK luabind::optional<luabind::mult<Vector3, double>> solve_ballistic_arc_lateral(lua_State *l, const Vector3 &projPos, double lateralSpeed, const Vector3 &tgt, double maxHeight);
		DLLNETWORK int abs_max(lua_State *l);

		DLLNETWORK float ease_in(float t, umath::EaseType type = umath::EaseType::Linear);
		DLLNETWORK float ease_out(float t, umath::EaseType type = umath::EaseType::Linear);
		DLLNETWORK float ease_in_out(float t, umath::EaseType type = umath::EaseType::Linear);

		DLLNETWORK double calc_horizontal_fov(double focalLengthInMM, double width, double height);
		DLLNETWORK double calc_vertical_fov(double focalLengthInMM, double width, double height);
		DLLNETWORK double calc_diagonal_fov(double focalLengthInMM, double width, double height);

		DLLNETWORK double horizontal_fov_to_vertical_fov(float fovDeg, float widthOrAspectRatio, float height = 1.f);
		DLLNETWORK int vertical_fov_to_horizontal_fov(float fovDeg, float widthOrAspectRatio, float height = 1.f);
		DLLNETWORK double diagonal_fov_to_vertical_fov(float diagonalFov, float aspectRatio);

		DLLNETWORK luabind::mult<float, float> get_frustum_plane_size(lua_State *l, float fovRad, float aspectRatio, float z);
		DLLNETWORK luabind::mult<luabind::tableT<Vector3>, float, float> get_frustum_plane_boundaries(lua_State *l, const Vector3 &pos, const Vector3 &forward, const Vector3 &up, float fovRad, float aspectRatio, float z);
		DLLNETWORK Vector3 get_frustum_plane_point(lua_State *l, const Vector3 &pos, const Vector3 &forward, const Vector3 &right, const Vector3 &up, float fovRad, float aspectRatio, const ::Vector2 &uv, float z);
	};
};

#endif

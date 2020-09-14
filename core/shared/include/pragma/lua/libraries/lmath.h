/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LMATH_H__
#define __LMATH_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace math
	{
		DLLNETWORK int randomf(lua_State *l);
		DLLNETWORK int normalize_angle(lua_State *l);
		DLLNETWORK int perlin_noise(lua_State *l);
		DLLNETWORK int sign(lua_State *l);
		DLLNETWORK int lerp(lua_State *l);
		DLLNETWORK int round(lua_State *l);
		DLLNETWORK int snap_to_grid(lua_State *l);
		DLLNETWORK int calc_hermite_spline(lua_State *l);
		DLLNETWORK int calc_hermite_spline_position(lua_State *l);
		DLLNETWORK int is_in_range(lua_State *l);
		DLLNETWORK int normalize_uv_coordinates(lua_State *l);

		DLLNETWORK int solve_quadric(lua_State *l);
		DLLNETWORK int solve_cubic(lua_State *l);
		DLLNETWORK int solve_quartic(lua_State *l);
		DLLNETWORK int calc_ballistic_velocity(lua_State *l);
		DLLNETWORK int calc_ballistic_time_of_flight(lua_State *l);
		DLLNETWORK int solve_ballistic_arc(lua_State *l);
		DLLNETWORK int solve_ballistic_arc_lateral(lua_State *l);
		DLLNETWORK int abs_max(lua_State *l);

		DLLNETWORK int ease_in(lua_State *l);
		DLLNETWORK int ease_out(lua_State *l);
		DLLNETWORK int ease_in_out(lua_State *l);

		DLLNETWORK int calc_horizontal_fov(lua_State *l);
		DLLNETWORK int calc_vertical_fov(lua_State *l);
		DLLNETWORK int calc_diagonal_fov(lua_State *l);

		DLLNETWORK int horizontal_fov_to_vertical_fov(lua_State *l);
		DLLNETWORK int vertical_fov_to_horizontal_fov(lua_State *l);
		DLLNETWORK int diagonal_fov_to_vertical_fov(lua_State *l);

		DLLNETWORK int get_frustum_plane_size(lua_State *l);
		DLLNETWORK int get_frustum_plane_boundaries(lua_State *l);
		DLLNETWORK int get_frustum_plane_point(lua_State *l);
		
		DLLNETWORK int calc_dielectric_specular_reflection(lua_State *l);
	};
};

#endif

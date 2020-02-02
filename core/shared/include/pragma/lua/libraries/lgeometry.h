#ifndef __LGEOMETRY_H__
#define __LGEOMETRY_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace geometry
	{
		DLLNETWORK int closest_point_on_aabb_to_point(lua_State *l);
		DLLNETWORK int closest_points_between_lines(lua_State *l);
		DLLNETWORK int closest_point_on_plane_to_point(lua_State *l);
		DLLNETWORK int closest_point_on_triangle_to_point(lua_State *l);
		DLLNETWORK int smallest_enclosing_sphere(lua_State *l);
		DLLNETWORK int closest_point_on_line_to_point(lua_State *l);
		DLLNETWORK int closest_point_on_sphere_to_line(lua_State *l);
		DLLNETWORK int get_triangle_winding_order(lua_State *l);
		DLLNETWORK int generate_truncated_cone_mesh(lua_State *l);
		DLLNETWORK int calc_face_normal(lua_State *l);

		DLLNETWORK int calc_volume_of_triangle(lua_State *l);
		DLLNETWORK int calc_volume_of_polyhedron(lua_State *l);
		DLLNETWORK int calc_center_of_mass(lua_State *l);
		DLLNETWORK int calc_triangle_area(lua_State *l);

		DLLNETWORK int calc_barycentric_coordinates(lua_State *l);
		DLLNETWORK int calc_rotation_between_planes(lua_State *l);
		DLLNETWORK int get_side_of_point_to_line(lua_State *l);
		DLLNETWORK int get_side_of_point_to_plane(lua_State *l);
		DLLNETWORK int get_outline_vertices(lua_State *l);
		DLLNETWORK int triangulate_point_cloud(lua_State *l);
		DLLNETWORK int triangulate(lua_State *l);
	};
};

#endif

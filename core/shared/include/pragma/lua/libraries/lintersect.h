#ifndef __LINTERSECT_H__
#define __LINTERSECT_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace intersect
	{
		DLLNETWORK int aabb_with_aabb(lua_State *l);
		DLLNETWORK int aabb_with_plane(lua_State *l);
		DLLNETWORK int aabb_with_triangle(lua_State *l);
		DLLNETWORK int obb_with_plane(lua_State *l);
		DLLNETWORK int sphere_with_plane(lua_State *l);
		DLLNETWORK int sphere_with_sphere(lua_State *l);
		DLLNETWORK int aabb_with_sphere(lua_State *l);
		DLLNETWORK int line_aabb(lua_State *l);
		DLLNETWORK int line_plane(lua_State *l);
		DLLNETWORK int vector_in_bounds(lua_State *l);
		DLLNETWORK int point_in_plane_mesh(lua_State *l);
		DLLNETWORK int line_obb(lua_State *l);
		DLLNETWORK int line_mesh(lua_State *l);
		DLLNETWORK int sphere_in_plane_mesh(lua_State *l);
		DLLNETWORK int aabb_in_plane_mesh(lua_State *l);
		DLLNETWORK int sphere_with_cone(lua_State *l);
		DLLNETWORK int line_triangle(lua_State *l);
		DLLNETWORK int line_line(lua_State *l);
	};
};

#endif

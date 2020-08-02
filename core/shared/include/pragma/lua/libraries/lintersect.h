/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LINTERSECT_H__
#define __LINTERSECT_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace intersect
	{
		DLLNETWORK void line_aabb(lua_State *l,const Vector3 &start,const Vector3 &dir,const Vector3 &min,const Vector3 &max,luabind::object &outMin,luabind::object &outMax);
		DLLNETWORK luabind::object line_plane(lua_State *l,const Vector3 &origin,const Vector3 &dir,const Vector3 &n,float d);
		DLLNETWORK bool point_in_plane_mesh(lua_State *l,const Vector3 &vec,luabind::table<> planeTable);
		DLLNETWORK luabind::object line_obb(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,const Vector3 &min,const Vector3 &max,bool precise=false,const umath::Transform &pose={});
		DLLNETWORK void line_mesh(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,ModelSubMesh &mesh,luabind::object &r0,luabind::object &r1,bool precise=false,const umath::Transform &meshPose={});
		DLLNETWORK void line_mesh(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,ModelMesh &mesh,luabind::object &r0,luabind::object &r1,bool precise=false,const umath::Transform &meshPose={});
		DLLNETWORK void line_mesh(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,Model &mdl,uint32_t lod,luabind::object &r0,luabind::object &r1,bool precise=false,const umath::Transform &meshPose={});
		DLLNETWORK void line_mesh(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,Model &mdl,luabind::table<> tBodyGroups,luabind::object &r0,luabind::object &r1,bool precise=false,const umath::Transform &meshPose={});
		DLLNETWORK int sphere_in_plane_mesh(lua_State *l,const Vector3 &vec,float r,luabind::table<> planeTable);
		DLLNETWORK int aabb_in_plane_mesh(lua_State *l,const Vector3 &min,const Vector3 &max,luabind::table<> planeTable);
		DLLNETWORK void line_triangle(lua_State *l,const Vector3 &lineOrigin,const Vector3 &lineDir,const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,luabind::object &outT,luabind::object &outUv,bool cull=false);
		DLLNETWORK luabind::object line_line(lua_State *l,const ::Vector2 &start0,const ::Vector2 &end0,const ::Vector2 &start1,const ::Vector2 &end1);
	};
};

#endif

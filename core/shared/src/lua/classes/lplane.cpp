/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lplane.h"
#include "luasystem.h"
#include <mathutil/plane.hpp>

DLLNETWORK void Lua_Plane_GetNormal(lua_State *l,umath::Plane &plane)
{
	Lua::Push<Vector3>(l,plane.GetNormal());
}
DLLNETWORK void Lua_Plane_GetPos(lua_State *l,umath::Plane &plane)
{
	Lua::Push<Vector3>(l,plane.GetPos());
}
DLLNETWORK void Lua_Plane_GetDistance(lua_State *l,umath::Plane &plane)
{
	Lua::PushNumber(l,plane.GetDistance());
}
DLLNETWORK void Lua_Plane_GetDistance(lua_State *l,umath::Plane &plane,const Vector3 &pos)
{
	Lua::PushNumber(l,plane.GetDistance(pos));
}
DLLNETWORK void Lua_Plane_MoveToPos(lua_State*,umath::Plane &plane,Vector3 &pos)
{
	plane.MoveToPos(pos);
}
DLLNETWORK void Lua_Plane_Rotate(lua_State*,umath::Plane &plane,EulerAngles &ang)
{
	plane.Rotate(ang);
}
DLLNETWORK void Lua_Plane_GetCenterPos(lua_State *l,umath::Plane &plane)
{
	Lua::Push<Vector3>(l,plane.GetCenterPos());
}
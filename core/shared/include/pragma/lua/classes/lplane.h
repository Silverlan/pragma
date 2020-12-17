/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LPLANE_H__
#define __LPLANE_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace umath {class Plane;};
DLLNETWORK void Lua_Plane_GetNormal(lua_State *l,umath::Plane &plane);
DLLNETWORK void Lua_Plane_GetPos(lua_State *l,umath::Plane &plane);
DLLNETWORK void Lua_Plane_GetDistance(lua_State *l,umath::Plane &plane);
DLLNETWORK void Lua_Plane_GetDistance(lua_State *l,umath::Plane &plane,const Vector3 &pos);
DLLNETWORK void Lua_Plane_MoveToPos(lua_State *l,umath::Plane &plane,Vector3 &pos);
DLLNETWORK void Lua_Plane_Rotate(lua_State *l,umath::Plane &plane,EulerAngles &ang);
DLLNETWORK void Lua_Plane_GetCenterPos(lua_State *l,umath::Plane &plane);

#endif
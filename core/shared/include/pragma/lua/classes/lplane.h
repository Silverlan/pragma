#ifndef __LPLANE_H__
#define __LPLANE_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/math/plane.h"

DLLNETWORK void Lua_Plane_GetNormal(lua_State *l,Plane &plane);
DLLNETWORK void Lua_Plane_GetPos(lua_State *l,Plane &plane);
DLLNETWORK void Lua_Plane_GetDistance(lua_State *l,Plane &plane);
DLLNETWORK void Lua_Plane_GetDistance(lua_State *l,Plane &plane,const Vector3 &pos);
DLLNETWORK void Lua_Plane_MoveToPos(lua_State *l,Plane &plane,Vector3 &pos);
DLLNETWORK void Lua_Plane_Rotate(lua_State *l,Plane &plane,EulerAngles &ang);
DLLNETWORK void Lua_Plane_GetCenterPos(lua_State *l,Plane &plane);

#endif
#include "stdafx_shared.h"
#include "pragma/lua/classes/lplane.h"
#include "luasystem.h"

DLLNETWORK void Lua_Plane_GetNormal(lua_State *l,Plane &plane)
{
	Lua::Push<Vector3>(l,plane.GetNormal());
}
DLLNETWORK void Lua_Plane_GetPos(lua_State *l,Plane &plane)
{
	Lua::Push<Vector3>(l,plane.GetPos());
}
DLLNETWORK void Lua_Plane_GetDistance(lua_State *l,Plane &plane)
{
	Lua::PushNumber(l,plane.GetDistance());
}
DLLNETWORK void Lua_Plane_GetDistance(lua_State *l,Plane &plane,const Vector3 &pos)
{
	Lua::PushNumber(l,plane.GetDistance(pos));
}
DLLNETWORK void Lua_Plane_MoveToPos(lua_State*,Plane &plane,Vector3 &pos)
{
	plane.MoveToPos(pos);
}
DLLNETWORK void Lua_Plane_Rotate(lua_State*,Plane &plane,EulerAngles &ang)
{
	plane.Rotate(ang);
}
DLLNETWORK void Lua_Plane_GetCenterPos(lua_State *l,Plane &plane)
{
	Lua::Push<Vector3>(l,plane.GetCenterPos());
}
#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
//#include "s_basewheel.h"
#include "pragma/lua/classes/s_lvhcwheel.h"
#include "luasystem.h"
// COMPONENT TODO
/*void Lua_VHCWheel_SetFrontWheel(lua_State *l,VHCWheelHandle &hEnt,Bool bFrontWheel)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetFrontWheel(bFrontWheel);
}
void Lua_VHCWheel_SetChassisConnectionPoint(lua_State *l,VHCWheelHandle &hEnt,const Vector3 &conPoint)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetChassisConnectionPoint(conPoint);
}
void Lua_VHCWheel_SetWheelAxle(lua_State *l,VHCWheelHandle &hEnt,const Vector3 &axle)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetWheelAxle(axle);
}
void Lua_VHCWheel_SetWheelDirection(lua_State *l,VHCWheelHandle &hEnt,const Vector3 &dir)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetWheelDirection(dir);
}
void Lua_VHCWheel_SetMaxSuspensionLength(lua_State *l,VHCWheelHandle &hEnt,Float len)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetMaxSuspensionLength(len);
}
void Lua_VHCWheel_SetMaxSuspensionCompression(lua_State *l,VHCWheelHandle &hEnt,Float cmp)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetMaxSuspensionCompression(cmp);
}
void Lua_VHCWheel_SetMaxDampingRelaxation(lua_State *l,VHCWheelHandle &hEnt,Float damping)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetMaxDampingRelaxation(damping);
}
void Lua_VHCWheel_SetWheelRadius(lua_State *l,VHCWheelHandle &hEnt,Float radius)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetWheelRadius(radius);
}
void Lua_VHCWheel_SetSuspensionStiffness(lua_State *l,VHCWheelHandle &hEnt,Float stiffness)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetSuspensionStiffness(stiffness);
}
void Lua_VHCWheel_SetWheelDampingCompression(lua_State *l,VHCWheelHandle &hEnt,Float damping)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetWheelDampingCompression(damping);
}
void Lua_VHCWheel_SetFrictionSlip(lua_State *l,VHCWheelHandle &hEnt,Float slip)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetFrictionSlip(slip);
}
void Lua_VHCWheel_SetSteeringAngle(lua_State *l,VHCWheelHandle &hEnt,Float angle)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetSteeringAngle(angle);
}
void Lua_VHCWheel_SetWheelRotation(lua_State *l,VHCWheelHandle &hEnt,Float rot)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetWheelRotation(rot);
}
void Lua_VHCWheel_SetRollInfluence(lua_State *l,VHCWheelHandle &hEnt,Float roll)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<SBaseWheel*>(hEnt.get())->SetRollInfluence(roll);
}*/
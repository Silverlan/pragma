/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/lentity_components.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/lua_entity_component.hpp"

namespace Lua
{
	namespace Gravity
	{
		static void SetGravityScale(lua_State *l,GravityHandle &hEnt,float scale);
		static void SetGravityOverride(lua_State *l,GravityHandle &hEnt,const Vector3 &dir,float gravity);
		static void SetGravityOverride(lua_State *l,GravityHandle &hEnt,const Vector3 &dir);
		static void SetGravityOverride(lua_State *l,GravityHandle &hEnt,float gravity);
		static void SetGravityOverride(lua_State *l,GravityHandle &hEnt);
		static void GetGravityScale(lua_State *l,GravityHandle &hEnt);
		static void HasGravityForceOverride(lua_State *l,GravityHandle &hEnt);
		static void HasGravityDirectionOverride(lua_State *l,GravityHandle &hEnt);
		static void GetGravityDirection(lua_State *l,GravityHandle &hEnt);
		static void GetGravity(lua_State *l,GravityHandle &hEnt);
		static void GetGravityForce(lua_State *l,GravityHandle &hEnt);
		static void CalcBallisticVelocity(lua_State *l,GravityHandle &hEnt,const Vector3 &origin,const Vector3 &destPos,float fireAngle,float maxSpeed,float spread,float maxPitch,float maxYaw);
	};
};
void Lua::register_gravity_component(luabind::module_ &module)
{
	auto def = luabind::class_<GravityHandle,BaseEntityComponentHandle>("GravityComponent");
	def.def("SetGravityScale",&Lua::Gravity::SetGravityScale);
	def.def("SetGravityOverride",static_cast<void(*)(lua_State*,GravityHandle&,const Vector3&,float)>(&Lua::Gravity::SetGravityOverride));
	def.def("SetGravityOverride",static_cast<void(*)(lua_State*,GravityHandle&,const Vector3&)>(&Lua::Gravity::SetGravityOverride));
	def.def("SetGravityOverride",static_cast<void(*)(lua_State*,GravityHandle&,float)>(&Lua::Gravity::SetGravityOverride));
	def.def("SetGravityOverride",static_cast<void(*)(lua_State*,GravityHandle&)>(&Lua::Gravity::SetGravityOverride));
	def.def("HasGravityForceOverride",&Lua::Gravity::HasGravityForceOverride);
	def.def("HasGravityDirectionOverride",&Lua::Gravity::HasGravityDirectionOverride);
	def.def("GetGravityDirection",&Lua::Gravity::GetGravityDirection);
	def.def("GetGravity",&Lua::Gravity::GetGravity);
	def.def("GetGravityForce",&Lua::Gravity::GetGravityForce);
	def.def("CalcBallisticVelocity",&Lua::Gravity::CalcBallisticVelocity);
	module[def];
}

void Lua::Gravity::SetGravityScale(lua_State *l,GravityHandle &hEnt,float scale)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetGravityScale(scale);
}
void Lua::Gravity::SetGravityOverride(lua_State *l,GravityHandle &hEnt,const Vector3 &dir,float gravity)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetGravityOverride(dir,gravity);
}
void Lua::Gravity::SetGravityOverride(lua_State *l,GravityHandle &hEnt,const Vector3 &dir)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetGravityOverride(dir);
}
void Lua::Gravity::SetGravityOverride(lua_State *l,GravityHandle &hEnt,float gravity)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetGravityOverride(gravity);
}
void Lua::Gravity::SetGravityOverride(lua_State *l,GravityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetGravityOverride();
}
void Lua::Gravity::GetGravityScale(lua_State *l,GravityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushNumber(l,hEnt->GetGravityScale());
}
void Lua::Gravity::HasGravityForceOverride(lua_State *l,GravityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->HasGravityForceOverride());
}
void Lua::Gravity::HasGravityDirectionOverride(lua_State *l,GravityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->HasGravityDirectionOverride());
}
void Lua::Gravity::GetGravityDirection(lua_State *l,GravityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::Push<Vector3>(l,hEnt->GetGravityDirection());
}
void Lua::Gravity::GetGravity(lua_State *l,GravityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushNumber(l,hEnt->GetGravity());
}
void Lua::Gravity::GetGravityForce(lua_State *l,GravityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::Push<Vector3>(l,hEnt->GetGravityForce());
}
void Lua::Gravity::CalcBallisticVelocity(lua_State *l,GravityHandle &hEnt,const Vector3 &origin,const Vector3 &destPos,float fireAngle,float maxSpeed,float spread,float maxPitch,float maxYaw)
{
	pragma::Lua::check_component(l,hEnt);
	Vector3 vel;
	auto b = hEnt->CalcBallisticVelocity(origin,destPos,fireAngle,maxSpeed,spread,maxPitch,maxYaw,vel);
	Lua::PushBool(l,b);
	if(b == true)
		Lua::Push<Vector3>(l,vel);
}

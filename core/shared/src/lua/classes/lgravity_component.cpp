/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/lentity_components.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/lua_entity_component.hpp"

namespace Lua
{
	namespace Gravity
	{
		static void CalcBallisticVelocity(lua_State *l,GravityHandle &hEnt,const Vector3 &origin,const Vector3 &destPos,float fireAngle,float maxSpeed,float spread,float maxPitch,float maxYaw);
	};
};
void Lua::register_gravity_component(luabind::module_ &module)
{
	auto def = luabind::class_<pragma::GravityComponent,pragma::BaseEntityComponent>("GravityComponent");
	def.def("SetGravityScale",&pragma::GravityComponent::SetGravityScale);
	def.def("SetGravityOverride",static_cast<void(pragma::GravityComponent::*)(const Vector3&,float)>(&pragma::GravityComponent::SetGravityOverride));
	def.def("SetGravityOverride",static_cast<void(pragma::GravityComponent::*)(const Vector3&)>(&pragma::GravityComponent::SetGravityOverride));
	def.def("SetGravityOverride",static_cast<void(pragma::GravityComponent::*)(float)>(&pragma::GravityComponent::SetGravityOverride));
	def.def("SetGravityOverride",static_cast<void(pragma::GravityComponent::*)()>(&pragma::GravityComponent::SetGravityOverride));
	def.def("HasGravityForceOverride",&pragma::GravityComponent::HasGravityForceOverride);
	def.def("HasGravityDirectionOverride",&pragma::GravityComponent::HasGravityDirectionOverride);
	def.def("GetGravityDirection",&pragma::GravityComponent::GetGravityDirection);
	def.def("GetGravity",&pragma::GravityComponent::GetGravity);
	def.def("GetGravityForce",&pragma::GravityComponent::GetGravityForce);
	def.def("CalcBallisticVelocity",&Lua::Gravity::CalcBallisticVelocity);
	module[def];
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

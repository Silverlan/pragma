/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_weapon_component.hpp"
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/lentity_components_base_types.hpp>

namespace Lua
{
	namespace Weapon
	{
		namespace Client
		{
			static void PlayViewActivity(lua_State *l,CWeaponHandle &hWep,int activity,uint32_t flags);
			static void PlayViewActivity(lua_State *l,CWeaponHandle &hWep,int activity);
			static void SetViewModel(lua_State *l,CWeaponHandle &hWep,const std::string &mdl);
			static void GetViewModel(lua_State *l,CWeaponHandle &hWep);
			static void SetHideWorldModelInFirstPerson(lua_State *l,CWeaponHandle &hWep,bool b);
			static void GetHideWorldModelInFirstPerson(lua_State *l,CWeaponHandle &hWep);
			static void IsInFirstPersonMode(lua_State *l,CWeaponHandle &hWep);
			static void SetViewModelOffset(lua_State *l,CWeaponHandle &hWep,const Vector3 &offset);
			static void GetViewModelOffset(lua_State *l,CWeaponHandle &hWep);
			static void SetViewFOV(lua_State *l,CWeaponHandle &hWep,float fov);
			static void GetViewFOV(lua_State *l,CWeaponHandle &hWep);
		};
	};
};
void Lua::register_cl_weapon_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<CWeaponHandle,BaseEntityComponentHandle>("WeaponComponent");
	Lua::register_base_weapon_component_methods<luabind::class_<CWeaponHandle,BaseEntityComponentHandle>,CWeaponHandle>(l,def);
	def.def("PlayViewActivity",static_cast<void(*)(lua_State*,CWeaponHandle&,int,uint32_t)>(&Lua::Weapon::Client::PlayViewActivity));
	def.def("PlayViewActivity",static_cast<void(*)(lua_State*,CWeaponHandle&,int)>(&Lua::Weapon::Client::PlayViewActivity));
	def.def("SetViewModel",&Lua::Weapon::Client::SetViewModel);
	def.def("GetViewModel",&Lua::Weapon::Client::GetViewModel);
	def.def("SetHideWorldModelInFirstPerson",&Lua::Weapon::Client::SetHideWorldModelInFirstPerson);
	def.def("GetHideWorldModelInFirstPerson",&Lua::Weapon::Client::GetHideWorldModelInFirstPerson);
	def.def("IsInFirstPersonMode",&Lua::Weapon::Client::IsInFirstPersonMode);
	def.def("SetViewModelOffset",&Lua::Weapon::Client::SetViewModelOffset);
	def.def("GetViewModelOffset",&Lua::Weapon::Client::GetViewModelOffset);
	def.def("SetViewFOV",&Lua::Weapon::Client::SetViewFOV);
	def.def("GetViewFOV",&Lua::Weapon::Client::GetViewFOV);
	def.add_static_constant("EVENT_ATTACH_TO_OWNER",pragma::CWeaponComponent::EVENT_ATTACH_TO_OWNER);
	module[def];
}
void Lua::Weapon::Client::PlayViewActivity(lua_State *l,CWeaponHandle &hWep,int activity,uint32_t flags)
{
	pragma::Lua::check_component(l,hWep);
	static_cast<pragma::CWeaponComponent&>(*hWep.get()).PlayViewActivity(static_cast<Activity>(activity),static_cast<pragma::FPlayAnim>(flags));
}

void Lua::Weapon::Client::PlayViewActivity(lua_State *l,CWeaponHandle &hWep,int activity)
{
	pragma::Lua::check_component(l,hWep);
	static_cast<pragma::CWeaponComponent&>(*hWep.get()).PlayViewActivity(static_cast<Activity>(activity));
}

void Lua::Weapon::Client::SetViewModel(lua_State *l,CWeaponHandle &hWep,const std::string &mdl)
{
	pragma::Lua::check_component(l,hWep);
	static_cast<pragma::CWeaponComponent&>(*hWep.get()).SetViewModel(mdl);
}

void Lua::Weapon::Client::GetViewModel(lua_State *l,CWeaponHandle &hWep)
{
	pragma::Lua::check_component(l,hWep);
	auto &vmModelName = static_cast<pragma::CWeaponComponent&>(*hWep.get()).GetViewModelName();
	if(vmModelName.has_value() == false)
		return;
	Lua::PushString(l,*vmModelName);
}

void Lua::Weapon::Client::SetHideWorldModelInFirstPerson(lua_State *l,CWeaponHandle &hWep,bool b)
{
	pragma::Lua::check_component(l,hWep);
	static_cast<pragma::CWeaponComponent&>(*hWep.get()).SetHideWorldModelInFirstPerson(b);
}
void Lua::Weapon::Client::GetHideWorldModelInFirstPerson(lua_State *l,CWeaponHandle &hWep)
{
	pragma::Lua::check_component(l,hWep);
	Lua::PushBool(l,static_cast<pragma::CWeaponComponent&>(*hWep.get()).GetHideWorldModelInFirstPerson());
}
void Lua::Weapon::Client::IsInFirstPersonMode(lua_State *l,CWeaponHandle &hWep)
{
	pragma::Lua::check_component(l,hWep);
	Lua::PushBool(l,static_cast<pragma::CWeaponComponent&>(*hWep.get()).IsInFirstPersonMode());
}
void Lua::Weapon::Client::SetViewModelOffset(lua_State *l,CWeaponHandle &hWep,const Vector3 &offset)
{
	pragma::Lua::check_component(l,hWep);
	static_cast<pragma::CWeaponComponent&>(*hWep.get()).SetViewModelOffset(offset);
}
void Lua::Weapon::Client::GetViewModelOffset(lua_State *l,CWeaponHandle &hWep)
{
	pragma::Lua::check_component(l,hWep);
	Lua::Push<Vector3>(l,static_cast<pragma::CWeaponComponent&>(*hWep.get()).GetViewModelOffset());
}
void Lua::Weapon::Client::SetViewFOV(lua_State *l,CWeaponHandle &hWep,float fov)
{
	pragma::Lua::check_component(l,hWep);
	static_cast<pragma::CWeaponComponent&>(*hWep.get()).SetViewFOV(fov);
}
void Lua::Weapon::Client::GetViewFOV(lua_State *l,CWeaponHandle &hWep)
{
	pragma::Lua::check_component(l,hWep);
	Lua::PushNumber(l,static_cast<pragma::CWeaponComponent&>(*hWep.get()).GetViewFOV());
}


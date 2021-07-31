/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
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
			static void PlayViewActivity(lua_State *l,pragma::CWeaponComponent &hWep,int activity,uint32_t flags);
			static void PlayViewActivity(lua_State *l,pragma::CWeaponComponent &hWep,int activity);
			static void SetViewModel(lua_State *l,pragma::CWeaponComponent &hWep,const std::string &mdl);
			static void GetViewModel(lua_State *l,pragma::CWeaponComponent &hWep);
			static void SetHideWorldModelInFirstPerson(lua_State *l,pragma::CWeaponComponent &hWep,bool b);
			static void GetHideWorldModelInFirstPerson(lua_State *l,pragma::CWeaponComponent &hWep);
			static void IsInFirstPersonMode(lua_State *l,pragma::CWeaponComponent &hWep);
			static void SetViewModelOffset(lua_State *l,pragma::CWeaponComponent &hWep,const Vector3 &offset);
			static void GetViewModelOffset(lua_State *l,pragma::CWeaponComponent &hWep);
			static void SetViewFOV(lua_State *l,pragma::CWeaponComponent &hWep,float fov);
			static void GetViewFOV(lua_State *l,pragma::CWeaponComponent &hWep);
		};
	};
};
void Lua::register_cl_weapon_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<pragma::CWeaponComponent,pragma::BaseWeaponComponent>("WeaponComponent");
	def.def("PlayViewActivity",&pragma::CWeaponComponent::PlayViewActivity);
	def.def("PlayViewActivity",static_cast<bool(*)(pragma::CWeaponComponent &wepComponent,Activity)>([](pragma::CWeaponComponent &wepComponent,Activity activity) {
		return wepComponent.PlayViewActivity(activity);
	}));
	def.def("SetViewModel",&pragma::CWeaponComponent::SetViewModel);
	def.def("GetViewModel",&pragma::CWeaponComponent::GetViewModel);
	def.def("SetHideWorldModelInFirstPerson",&pragma::CWeaponComponent::SetHideWorldModelInFirstPerson);
	def.def("GetHideWorldModelInFirstPerson",&pragma::CWeaponComponent::GetHideWorldModelInFirstPerson);
	def.def("IsInFirstPersonMode",&pragma::CWeaponComponent::IsInFirstPersonMode);
	def.def("SetViewModelOffset",&pragma::CWeaponComponent::SetViewModelOffset);
	def.def("GetViewModelOffset",&pragma::CWeaponComponent::GetViewModelOffset);
	def.def("SetViewFOV",&pragma::CWeaponComponent::SetViewFOV);
	def.def("GetViewFOV",&pragma::CWeaponComponent::GetViewFOV);
	def.add_static_constant("EVENT_ATTACH_TO_OWNER",pragma::CWeaponComponent::EVENT_ATTACH_TO_OWNER);
	module[def];
}

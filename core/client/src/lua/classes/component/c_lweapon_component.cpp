/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/c_viewmodel.h"
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

void Lua::register_cl_weapon_component(lua_State *l, luabind::module_ &module)
{
	auto def = pragma::lua::create_entity_component_class<pragma::CWeaponComponent, pragma::BaseWeaponComponent>("WeaponComponent");
	def.def("PlayViewActivity", &pragma::CWeaponComponent::PlayViewActivity);
	def.def("PlayViewActivity", static_cast<bool (*)(pragma::CWeaponComponent & wepComponent, Activity)>([](pragma::CWeaponComponent &wepComponent, Activity activity) { return wepComponent.PlayViewActivity(activity); }));
	def.def("SetViewModel", &pragma::CWeaponComponent::SetViewModel);
	def.def("GetViewModel", &pragma::CWeaponComponent::GetViewModel);
	def.def("SetHideWorldModelInFirstPerson", &pragma::CWeaponComponent::SetHideWorldModelInFirstPerson);
	def.def("GetHideWorldModelInFirstPerson", &pragma::CWeaponComponent::GetHideWorldModelInFirstPerson);
	def.def("IsInFirstPersonMode", &pragma::CWeaponComponent::IsInFirstPersonMode);
	def.def("SetViewModelOffset", &pragma::CWeaponComponent::SetViewModelOffset);
	def.def("GetViewModelOffset", &pragma::CWeaponComponent::GetViewModelOffset);
	def.def("SetViewFOV", &pragma::CWeaponComponent::SetViewFOV);
	def.def("GetViewFOV", &pragma::CWeaponComponent::GetViewFOV);
	def.add_static_constant("EVENT_ATTACH_TO_OWNER", pragma::CWeaponComponent::EVENT_ATTACH_TO_OWNER);
	module[def];
}

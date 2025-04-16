/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/entities/components/s_weapon_component.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

#include <pragma/lua/ostream_operator_alias.hpp>

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseWeaponComponent);

void Lua::register_sv_weapon_component(lua_State *l, luabind::module_ &module)
{
	auto def = pragma::lua::create_entity_component_class<pragma::SWeaponComponent, pragma::BaseWeaponComponent>("WeaponComponent");
	def.def("SetPrimaryClipSize", &pragma::SWeaponComponent::SetPrimaryClipSize);
	def.def("SetSecondaryClipSize", &pragma::SWeaponComponent::SetSecondaryClipSize);
	def.def("SetMaxPrimaryClipSize", &pragma::SWeaponComponent::SetMaxPrimaryClipSize);
	def.def("SetMaxSecondaryClipSize", &pragma::SWeaponComponent::SetMaxSecondaryClipSize);
	def.def("AddPrimaryClip", &pragma::SWeaponComponent::AddPrimaryClip);
	def.def("AddSecondaryClip", &pragma::SWeaponComponent::AddSecondaryClip);
	def.def("RefillPrimaryClip", static_cast<void (pragma::SWeaponComponent::*)(UInt16)>(&pragma::SWeaponComponent::RefillPrimaryClip));
	def.def("RefillSecondaryClip", static_cast<void (pragma::SWeaponComponent::*)(UInt16)>(&pragma::SWeaponComponent::RefillSecondaryClip));
	def.def("RefillPrimaryClip", static_cast<void (pragma::SWeaponComponent::*)()>(&pragma::SWeaponComponent::RefillPrimaryClip));
	def.def("RefillSecondaryClip", static_cast<void (pragma::SWeaponComponent::*)()>(&pragma::SWeaponComponent::RefillSecondaryClip));
	def.def("RemovePrimaryClip", static_cast<void (pragma::SWeaponComponent::*)(UInt16)>(&pragma::SWeaponComponent::RemovePrimaryClip));
	def.def("RemoveSecondaryClip", static_cast<void (pragma::SWeaponComponent::*)(UInt16)>(&pragma::SWeaponComponent::RemoveSecondaryClip));
	def.def("RemovePrimaryClip", static_cast<void (*)(pragma::SWeaponComponent &)>([](pragma::SWeaponComponent &wepComponent) { wepComponent.RemovePrimaryClip(); }));
	def.def("RemoveSecondaryClip", static_cast<void (*)(pragma::SWeaponComponent &)>([](pragma::SWeaponComponent &wepComponent) { wepComponent.RemoveSecondaryClip(); }));
	def.def("SetPrimaryAmmoType", static_cast<void (pragma::SWeaponComponent::*)(UInt32)>(&pragma::SWeaponComponent::SetPrimaryAmmoType));
	def.def("SetPrimaryAmmoType", static_cast<void (pragma::SWeaponComponent::*)(const std::string &)>(&pragma::SWeaponComponent::SetPrimaryAmmoType));
	def.def("SetSecondaryAmmoType", static_cast<void (pragma::SWeaponComponent::*)(UInt32)>(&pragma::SWeaponComponent::SetSecondaryAmmoType));
	def.def("SetSecondaryAmmoType", static_cast<void (pragma::SWeaponComponent::*)(const std::string &)>(&pragma::SWeaponComponent::SetSecondaryAmmoType));
	module[def];
}


#include "pragma/lua/ostream_operator_alias.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include "pragma/lua/luaapi.h"

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "luasystem.h"

import pragma.server.entities.components;

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

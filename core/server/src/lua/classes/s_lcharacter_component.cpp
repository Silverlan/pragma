/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/entities/components/s_weapon_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/ostream_operator_alias.hpp>
#include <pragma/physics/raytraces.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lentity_components.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/entities/components/base_actor_component.hpp>
#include <pragma/lua/policies/game_object_policy.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

namespace Lua {
	namespace Character {
		namespace Server {
			static void DropWeapon(lua_State *l, pragma::SCharacterComponent &hEnt, pragma::SWeaponComponent &hWep);
			static void RemoveWeapon(lua_State *l, pragma::SCharacterComponent &hEnt, pragma::SWeaponComponent &hWep);
			static void DeployWeapon(lua_State *l, pragma::SCharacterComponent &hEnt, pragma::SWeaponComponent &hWep);
		};
	};
	namespace Actor {
		namespace Server {
			static void SetFaction(lua_State *l, pragma::SCharacterComponent &hEnt, const std::string &factionName);
		};
	};
};

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEntityComponent);
#endif

void Lua::register_sv_character_component(lua_State *l, luabind::module_ &module)
{
	auto def = pragma::lua::create_entity_component_class<pragma::SCharacterComponent, pragma::BaseCharacterComponent>("CharacterComponent");
	def.def("GiveWeapon", static_cast<BaseEntity *(pragma::SCharacterComponent::*)(std::string)>(&pragma::SCharacterComponent::GiveWeapon));
	def.def("DropActiveWeapon", &pragma::SCharacterComponent::DropActiveWeapon);
	def.def("DropWeapon", static_cast<void (pragma::SCharacterComponent::*)(std::string)>(&pragma::SCharacterComponent::DropWeapon));
	def.def("DropWeapon", &Lua::Character::Server::DropWeapon);
	def.def("RemoveWeapon", static_cast<void (pragma::SCharacterComponent::*)(std::string)>(&pragma::SCharacterComponent::RemoveWeapon));
	def.def("RemoveWeapon", &Lua::Character::Server::RemoveWeapon);
	def.def("RemoveWeapons", &pragma::SCharacterComponent::RemoveWeapons);
	def.def("DeployWeapon", static_cast<void (pragma::SCharacterComponent::*)(const std::string &)>(&pragma::SCharacterComponent::DeployWeapon));
	def.def("DeployWeapon", &Lua::Character::Server::DeployWeapon);
	def.def("HolsterWeapon", &pragma::SCharacterComponent::HolsterWeapon);
	def.def("SelectNextWeapon", &pragma::SCharacterComponent::SelectNextWeapon);
	def.def("SelectPreviousWeapon", &pragma::SCharacterComponent::SelectPreviousWeapon);
	def.def("PrimaryAttack", &pragma::SCharacterComponent::PrimaryAttack);
	def.def("SecondaryAttack", &pragma::SCharacterComponent::SecondaryAttack);
	def.def("TertiaryAttack", &pragma::SCharacterComponent::TertiaryAttack);
	def.def("Attack4", &pragma::SCharacterComponent::Attack4);
	def.def("ReloadWeapon", &pragma::SCharacterComponent::ReloadWeapon);

	def.def("SetNoTarget", &pragma::SCharacterComponent::SetNoTarget);
	def.def("GetNoTarget", &pragma::SCharacterComponent::GetNoTarget);
	def.def("SetGodMode", &pragma::SCharacterComponent::SetGodMode);
	def.def("GetGodMode", &pragma::SCharacterComponent::GetGodMode);
	def.def("GetFaction", &pragma::SCharacterComponent::GetFaction);
	def.def("SetFaction", static_cast<void (*)(lua_State *, pragma::SCharacterComponent &, const std::string &)>(&Lua::Actor::Server::SetFaction));
	def.def("SetFaction", &pragma::SCharacterComponent::SetFaction);

	// This is a bit of a hack: Usually the client controls the view angles of the player, which means changing it serverside would
	// have no effect, however that would make these Lua-bindings useless. Instead, when changing the angles of a player through Lua,
	// we'll redirect it to another function which will forcibly overwrite the client angles.
	def.def(
	  "SetViewAngles", +[](pragma::SCharacterComponent &c, const EulerAngles &ang) {
		  auto hPl = c.GetEntity().GetPlayerComponent();
		  if(hPl.valid())
			  static_cast<pragma::SPlayerComponent *>(hPl.get())->SetViewRotation(uquat::create(ang));
	  });
	def.def(
	  "SetViewRotation", +[](pragma::SCharacterComponent &c, const Quat &rot) {
		  auto hPl = c.GetEntity().GetPlayerComponent();
		  if(hPl.valid())
			  static_cast<pragma::SPlayerComponent *>(hPl.get())->SetViewRotation(rot);
	  });

	module[def];
}
void Lua::Character::Server::DropWeapon(lua_State *l, pragma::SCharacterComponent &hEnt, pragma::SWeaponComponent &hWep) { hEnt.DropWeapon(&hWep.GetEntity()); }
void Lua::Character::Server::RemoveWeapon(lua_State *l, pragma::SCharacterComponent &hEnt, pragma::SWeaponComponent &hWep) { hEnt.RemoveWeapon(hWep.GetEntity()); }
void Lua::Character::Server::DeployWeapon(lua_State *l, pragma::SCharacterComponent &hEnt, pragma::SWeaponComponent &hWep) { hEnt.DeployWeapon(hWep.GetEntity()); }

/////////////

void Lua::Actor::Server::SetFaction(lua_State *l, pragma::SCharacterComponent &hEnt, const std::string &factionName)
{
	auto &factionManager = pragma::SAIComponent::GetFactionManager();
	auto faction = factionManager.FindFactionByName(factionName);
	if(faction == nullptr)
		return;
	hEnt.SetFaction(*faction);
}

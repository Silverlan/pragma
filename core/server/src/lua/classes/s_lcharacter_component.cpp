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
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/physics/raytraces.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lentity_components.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/entities/components/base_actor_component.hpp>

namespace Lua
{
	namespace Character
	{
		namespace Server
		{
			static void GiveWeapon(lua_State *l,SCharacterHandle &hEnt,const std::string &className);
			static void DropActiveWeapon(lua_State *l,SCharacterHandle &hEnt);
			static void DropWeapon(lua_State *l,SCharacterHandle &hEnt,const std::string &className);
			static void DropWeapon(lua_State *l,SCharacterHandle &hEnt,SWeaponHandle &hWep);
			static void RemoveWeapon(lua_State *l,SCharacterHandle &hEnt,const std::string &className);
			static void RemoveWeapon(lua_State *l,SCharacterHandle &hEnt,SWeaponHandle &hWep);
			static void RemoveWeapons(lua_State *l,SCharacterHandle &hEnt);
			static void DeployWeapon(lua_State *l,SCharacterHandle &hEnt,const std::string &className);
			static void DeployWeapon(lua_State *l,SCharacterHandle &hEnt,SWeaponHandle &hWep);
			static void HolsterWeapon(lua_State *l,SCharacterHandle &hEnt);
			static void SelectNextWeapon(lua_State *l,SCharacterHandle &hEnt);
			static void SelectPreviousWeapon(lua_State *l,SCharacterHandle &hEnt);

			static void PrimaryAttack(lua_State *l,SCharacterHandle &hEnt);
			static void SecondaryAttack(lua_State *l,SCharacterHandle &hEnt);
			static void TertiaryAttack(lua_State *l,SCharacterHandle &hEnt);
			static void Attack4(lua_State *l,SCharacterHandle &hEnt);
			static void ReloadWeapon(lua_State *l,SCharacterHandle &hEnt);
		};
	};
	namespace Actor
	{
		namespace Server
		{
			static void GetNoTarget(lua_State *l,SCharacterHandle &hEntity);
			static void SetNoTarget(lua_State *l,SCharacterHandle &hEntity,bool b);
			static void GetGodMode(lua_State *l,SCharacterHandle &hEntity);
			static void SetGodMode(lua_State *l,SCharacterHandle &hEntity,bool b);
			static void GetFaction(lua_State *l,SCharacterHandle &hEnt);
			static void SetFaction(lua_State *l,SCharacterHandle &hEnt,const std::string &factionName);
			static void SetFaction(lua_State *l,SCharacterHandle &hEnt,std::shared_ptr<::Faction> &faction);
		};
	};
};

void Lua::register_sv_character_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<SCharacterHandle,BaseEntityComponentHandle>("CharacterComponent");
	Lua::register_base_character_component_methods<luabind::class_<SCharacterHandle,BaseEntityComponentHandle>,SCharacterHandle>(l,def);
	def.def("GiveWeapon",&Lua::Character::Server::GiveWeapon);
	def.def("DropActiveWeapon",&Lua::Character::Server::DropActiveWeapon);
	def.def("DropWeapon",static_cast<void(*)(lua_State*,SCharacterHandle&,const std::string&)>(&Lua::Character::Server::DropWeapon));
	def.def("DropWeapon",static_cast<void(*)(lua_State*,SCharacterHandle&,SWeaponHandle&)>(&Lua::Character::Server::DropWeapon));
	def.def("RemoveWeapon",static_cast<void(*)(lua_State*,SCharacterHandle&,const std::string&)>(&Lua::Character::Server::RemoveWeapon));
	def.def("RemoveWeapon",static_cast<void(*)(lua_State*,SCharacterHandle&,SWeaponHandle&)>(&Lua::Character::Server::RemoveWeapon));
	def.def("RemoveWeapons",&Lua::Character::Server::RemoveWeapons);
	def.def("DeployWeapon",static_cast<void(*)(lua_State*,SCharacterHandle&,const std::string&)>(&Lua::Character::Server::DeployWeapon));
	def.def("DeployWeapon",static_cast<void(*)(lua_State*,SCharacterHandle&,SWeaponHandle&)>(&Lua::Character::Server::DeployWeapon));
	def.def("HolsterWeapon",&Lua::Character::Server::HolsterWeapon);
	def.def("SelectNextWeapon",&Lua::Character::Server::HolsterWeapon);
	def.def("SelectPreviousWeapon",&Lua::Character::Server::HolsterWeapon);
	def.def("PrimaryAttack",&Lua::Character::Server::PrimaryAttack);
	def.def("SecondaryAttack",&Lua::Character::Server::SecondaryAttack);
	def.def("TertiaryAttack",&Lua::Character::Server::TertiaryAttack);
	def.def("Attack4",&Lua::Character::Server::Attack4);
	def.def("ReloadWeapon",&Lua::Character::Server::ReloadWeapon);

	def.def("SetNoTarget",&Lua::Actor::Server::SetNoTarget);
	def.def("GetNoTarget",&Lua::Actor::Server::GetNoTarget);
	def.def("SetGodMode",&Lua::Actor::Server::SetGodMode);
	def.def("GetGodMode",&Lua::Actor::Server::GetGodMode);
	def.def("GetFaction",&Lua::Actor::Server::GetFaction);
	def.def("SetFaction",static_cast<void(*)(lua_State*,SCharacterHandle&,const std::string&)>(&Lua::Actor::Server::SetFaction));
	def.def("SetFaction",static_cast<void(*)(lua_State*,SCharacterHandle&,std::shared_ptr<::Faction>&)>(&Lua::Actor::Server::SetFaction));
	module[def];
}
void Lua::Character::Server::GiveWeapon(lua_State *l,SCharacterHandle &hEnt,const std::string &className)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->GiveWeapon(className);
}
void Lua::Character::Server::DropWeapon(lua_State *l,SCharacterHandle &hEnt,const std::string &className)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->DropWeapon(className);
}
void Lua::Character::Server::DropWeapon(lua_State *l,SCharacterHandle &hEnt,SWeaponHandle &hWep)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->DropWeapon(&hWep.get()->GetEntity());
}
void Lua::Character::Server::DropActiveWeapon(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->DropActiveWeapon();
}
void Lua::Character::Server::RemoveWeapon(lua_State *l,SCharacterHandle &hEnt,const std::string &className)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->RemoveWeapon(className);
}
void Lua::Character::Server::RemoveWeapon(lua_State *l,SCharacterHandle &hEnt,SWeaponHandle &hWep)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->RemoveWeapon(hWep.get()->GetEntity());
}
void Lua::Character::Server::RemoveWeapons(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->RemoveWeapons();
}
void Lua::Character::Server::DeployWeapon(lua_State *l,SCharacterHandle &hEnt,const std::string &className)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->DeployWeapon(className);
}
void Lua::Character::Server::DeployWeapon(lua_State *l,SCharacterHandle &hEnt,SWeaponHandle &hWep)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->DeployWeapon(hWep.get()->GetEntity());
}
void Lua::Character::Server::HolsterWeapon(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->HolsterWeapon();
}
void Lua::Character::Server::SelectNextWeapon(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SelectNextWeapon();
}
void Lua::Character::Server::SelectPreviousWeapon(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SelectPreviousWeapon();
}
void Lua::Character::Server::PrimaryAttack(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->PrimaryAttack();
}
void Lua::Character::Server::SecondaryAttack(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SecondaryAttack();
}
void Lua::Character::Server::TertiaryAttack(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->TertiaryAttack();
}
void Lua::Character::Server::Attack4(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->Attack4();
}
void Lua::Character::Server::ReloadWeapon(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->ReloadWeapon();
}

/////////////

void Lua::Actor::Server::GetNoTarget(lua_State *l,SCharacterHandle &hEntity)
{
	pragma::Lua::check_component(l,hEntity);
	Lua::PushBool(l,hEntity->GetNoTarget());
}
void Lua::Actor::Server::SetNoTarget(lua_State *l,SCharacterHandle &hEntity,bool b)
{
	pragma::Lua::check_component(l,hEntity);
	hEntity->SetNoTarget(b);
}
void Lua::Actor::Server::GetGodMode(lua_State *l,SCharacterHandle &hEntity)
{
	pragma::Lua::check_component(l,hEntity);
	Lua::PushBool(l,hEntity->GetGodMode());
}
void Lua::Actor::Server::SetGodMode(lua_State *l,SCharacterHandle &hEntity,bool b)
{
	pragma::Lua::check_component(l,hEntity);
	hEntity->SetGodMode(b);
}
void Lua::Actor::Server::GetFaction(lua_State *l,SCharacterHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto *faction = hEnt->GetFaction();
	if(faction == nullptr)
		return;
	Lua::Push<std::shared_ptr<::Faction>>(l,faction->shared_from_this());
}
void Lua::Actor::Server::SetFaction(lua_State *l,SCharacterHandle &hEnt,const std::string &factionName)
{
	pragma::Lua::check_component(l,hEnt);
	auto &factionManager = pragma::SAIComponent::GetFactionManager();
	auto faction = factionManager.FindFactionByName(factionName);
	if(faction == nullptr)
		return;
	hEnt->SetFaction(*faction);
}
void Lua::Actor::Server::SetFaction(lua_State *l,SCharacterHandle &hEnt,std::shared_ptr<::Faction> &faction)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetFaction(*faction);
}

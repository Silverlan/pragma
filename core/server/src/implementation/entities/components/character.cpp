// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.character;

import :ai;
import :entities;
import :entities.components.ai;
import :entities.components.player;
import :entities.components.weapon;

using namespace pragma;

void SCharacterComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { BaseCharacterComponent::RegisterEvents(componentManager, registerEvent); }
SCharacterComponent::SCharacterComponent(ecs::BaseEntity &ent) : BaseCharacterComponent(ent), m_faction(nullptr), m_bNoTarget(false), m_bGodMode(false) {}
void SCharacterComponent::Initialize()
{
	BaseCharacterComponent::Initialize();
	auto &pFrozenProp = GetFrozenProperty();
	auto hThis = GetHandle();
	pFrozenProp->AddCallback([hThis, this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> val) {
		if(hThis.valid() == false)
			return;
		OnFrozen(val);
	});
}
void SCharacterComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseCharacterComponent)); }
void SCharacterComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SCharacterComponent::OnTick(double tDelta) { BaseCharacterComponent::OnTick(tDelta); }

void SCharacterComponent::OnFrozen(bool bFrozen)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		p->Write<bool>(bFrozen);
		ent.SendNetEvent(m_netEvSetFrozen, p, networking::Protocol::SlowReliable);
	}
}

void SCharacterComponent::SetAmmoCount(UInt32 ammoType, UInt16 count)
{
	BaseCharacterComponent::SetAmmoCount(ammoType, count);
	auto &entThis = static_cast<SBaseEntity &>(GetEntity());
	if(entThis.IsShared()) {
		NetPacket p;
		p->Write<uint32_t>(ammoType);
		p->Write<uint16_t>(count);
		entThis.SendNetEvent(m_netEvSetAmmoCount, p, networking::Protocol::SlowReliable);
	}
}

void SCharacterComponent::SetActiveWeapon(ecs::BaseEntity *ent)
{
	BaseCharacterComponent::SetActiveWeapon(ent);
	auto &entThis = static_cast<SBaseEntity &>(GetEntity());
	if(entThis.IsShared()) {
		NetPacket p;
		networking::write_entity(p, ent);
		entThis.SendNetEvent(m_netEvSetActiveWeapon, p, networking::Protocol::SlowReliable);
	}
}

void SCharacterComponent::DropWeapon(std::string className)
{
	for(unsigned int i = 0; i < m_weapons.size(); i++) {
		EntityHandle &hEnt = m_weapons[i];
		if(hEnt.valid()) {
			ecs::BaseEntity *ent = hEnt.get();
			if(ent->GetClass() == className)
				DropWeapon(ent);
		}
	}
}
void SCharacterComponent::DropWeapon(ecs::BaseEntity *ent)
{
	if(m_weapons.empty())
		return;
	auto it = std::find_if(m_weapons.begin(), m_weapons.end(), [ent](EntityHandle &hEnt) { return (hEnt.valid() && hEnt.get() == ent) ? true : false; });
	if(it == m_weapons.end())
		return;
	auto *wep = it->get();
	m_weapons.erase(it);
	if(wep->IsWeapon())
		static_cast<SWeaponComponent *>(wep->GetWeaponComponent().get())->Drop();
	auto *wepActive = GetActiveWeapon();
	if(wep == wepActive)
		SetActiveWeapon(nullptr);
}

void SCharacterComponent::DropActiveWeapon()
{
	auto *wep = GetActiveWeapon();
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto wepComponent = wep->GetWeaponComponent();
	static_cast<SWeaponComponent *>(wepComponent.get())->Drop();
}

void SCharacterComponent::SelectNextWeapon()
{
	auto *wep = GetActiveWeapon();
	uint32_t wepIdx = 0;
	if(wep != nullptr) {
		auto bFound = false;
		auto it = std::find_if(m_weapons.begin(), m_weapons.end(), [wep](const EntityHandle &hWepOther) { return (wep == hWepOther.get()) ? true : false; });
		if(it != m_weapons.end()) {
			const auto fValid = [wep](const EntityHandle &hWepOther) { return hWepOther.valid(); };
			auto itNext = std::find_if(it + 1, m_weapons.end(), fValid);
			if(itNext == m_weapons.end())
				itNext = std::find_if(m_weapons.begin(), it - 1, fValid);
			if(itNext != m_weapons.end())
				wepIdx = itNext - m_weapons.begin();
		}
	}
	if(wepIdx >= m_weapons.size())
		return;
	DeployWeapon(*m_weapons[wepIdx].get());
}
void SCharacterComponent::SelectPreviousWeapon()
{
	auto *wep = GetActiveWeapon();
	uint32_t wepIdx = 0;
	if(wep != nullptr) {
		auto bFound = false;
		auto it = std::find_if(m_weapons.rbegin(), m_weapons.rend(), [wep](const EntityHandle &hWepOther) { return (wep == hWepOther.get()) ? true : false; });
		if(it != m_weapons.rend()) {
			const auto fValid = [wep](const EntityHandle &hWepOther) { return hWepOther.valid(); };
			auto itNext = std::find_if(it + 1, m_weapons.rend(), fValid);
			if(itNext == m_weapons.rend())
				itNext = std::find_if(m_weapons.rbegin(), it - 1, fValid);
			if(itNext != m_weapons.rend())
				wepIdx = m_weapons.size() - 1 - (itNext - m_weapons.rbegin());
		}
	}
	if(wepIdx >= m_weapons.size())
		return;
	DeployWeapon(*m_weapons[wepIdx].get());
}

void SCharacterComponent::SetNoTarget(bool b) { m_bNoTarget = b; }
bool SCharacterComponent::GetNoTarget() const { return m_bNoTarget; }
void SCharacterComponent::SetGodMode(bool b) { m_bGodMode = b; }
bool SCharacterComponent::GetGodMode() const { return m_bGodMode; }

Faction *SCharacterComponent::GetFaction() { return m_faction; }
void SCharacterComponent::SetFaction(Faction &faction)
{
	m_faction = &faction;
	// Apply relationships
}

namespace Lua {
	namespace Character {
		namespace Server {
			static void DropWeapon(lua::State *l, SCharacterComponent &hEnt, SWeaponComponent &hWep);
			static void RemoveWeapon(lua::State *l, SCharacterComponent &hEnt, SWeaponComponent &hWep);
			static void DeployWeapon(lua::State *l, SCharacterComponent &hEnt, SWeaponComponent &hWep);
		};
	};
	namespace Actor {
		namespace Server {
			static void SetFaction(lua::State *l, SCharacterComponent &hEnt, const std::string &factionName);
		};
	};
};

void SCharacterComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseCharacterComponent::RegisterLuaBindings(l, modEnts);

	auto def = pragma::LuaCore::create_entity_component_class<SCharacterComponent, BaseCharacterComponent>("CharacterComponent");
	def.def("GiveWeapon", static_cast<ecs::BaseEntity *(SCharacterComponent::*)(std::string)>(&SCharacterComponent::GiveWeapon));
	def.def("DropActiveWeapon", &SCharacterComponent::DropActiveWeapon);
	def.def("DropWeapon", static_cast<void (SCharacterComponent::*)(std::string)>(&SCharacterComponent::DropWeapon));
	def.def("DropWeapon", &Lua::Character::Server::DropWeapon);
	def.def("RemoveWeapon", static_cast<void (SCharacterComponent::*)(std::string)>(&SCharacterComponent::RemoveWeapon));
	def.def("RemoveWeapon", &Lua::Character::Server::RemoveWeapon);
	def.def("RemoveWeapons", &SCharacterComponent::RemoveWeapons);
	def.def("DeployWeapon", static_cast<void (SCharacterComponent::*)(const std::string &)>(&SCharacterComponent::DeployWeapon));
	def.def("DeployWeapon", &Lua::Character::Server::DeployWeapon);
	def.def("HolsterWeapon", &SCharacterComponent::HolsterWeapon);
	def.def("SelectNextWeapon", &SCharacterComponent::SelectNextWeapon);
	def.def("SelectPreviousWeapon", &SCharacterComponent::SelectPreviousWeapon);
	def.def("PrimaryAttack", &SCharacterComponent::PrimaryAttack);
	def.def("SecondaryAttack", &SCharacterComponent::SecondaryAttack);
	def.def("TertiaryAttack", &SCharacterComponent::TertiaryAttack);
	def.def("Attack4", &SCharacterComponent::Attack4);
	def.def("ReloadWeapon", &SCharacterComponent::ReloadWeapon);

	def.def("SetNoTarget", &SCharacterComponent::SetNoTarget);
	def.def("GetNoTarget", &SCharacterComponent::GetNoTarget);
	def.def("SetGodMode", &SCharacterComponent::SetGodMode);
	def.def("GetGodMode", &SCharacterComponent::GetGodMode);
	def.def("GetFaction", &SCharacterComponent::GetFaction);
	def.def("SetFaction", static_cast<void (*)(lua::State *, SCharacterComponent &, const std::string &)>(&Lua::Actor::Server::SetFaction));
	def.def("SetFaction", &SCharacterComponent::SetFaction);

	// This is a bit of a hack: Usually the client controls the view angles of the player, which means changing it serverside would
	// have no effect, however that would make these Lua-bindings useless. Instead, when changing the angles of a player through Lua,
	// we'll redirect it to another function which will forcibly overwrite the client angles.
	def.def(
	  "SetViewAngles", +[](SCharacterComponent &c, const EulerAngles &ang) {
		  auto hPl = c.GetEntity().GetPlayerComponent();
		  if(hPl.valid())
			  static_cast<SPlayerComponent *>(hPl.get())->SetViewRotation(uquat::create(ang));
	  });
	def.def(
	  "SetViewRotation", +[](SCharacterComponent &c, const Quat &rot) {
		  auto hPl = c.GetEntity().GetPlayerComponent();
		  if(hPl.valid())
			  static_cast<SPlayerComponent *>(hPl.get())->SetViewRotation(rot);
	  });

	modEnts[def];
}

void Lua::Character::Server::DropWeapon(lua::State *l, SCharacterComponent &hEnt, SWeaponComponent &hWep) { hEnt.DropWeapon(&hWep.GetEntity()); }
void Lua::Character::Server::RemoveWeapon(lua::State *l, SCharacterComponent &hEnt, SWeaponComponent &hWep) { hEnt.RemoveWeapon(hWep.GetEntity()); }
void Lua::Character::Server::DeployWeapon(lua::State *l, SCharacterComponent &hEnt, SWeaponComponent &hWep) { hEnt.DeployWeapon(hWep.GetEntity()); }

/////////////

void Lua::Actor::Server::SetFaction(lua::State *l, SCharacterComponent &hEnt, const std::string &factionName)
{
	auto &factionManager = SAIComponent::GetFactionManager();
	auto faction = factionManager.FindFactionByName(factionName);
	if(faction == nullptr)
		return;
	hEnt.SetFaction(*faction);
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_weapon_component.hpp"
#include "pragma/ai/s_disposition.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

void SCharacterComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { BaseCharacterComponent::RegisterEvents(componentManager, registerEvent); }
SCharacterComponent::SCharacterComponent(BaseEntity &ent) : BaseCharacterComponent(ent), m_faction(nullptr), m_bNoTarget(false), m_bGodMode(false) {}
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
void SCharacterComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SCharacterComponent::OnTick(double tDelta) { BaseCharacterComponent::OnTick(tDelta); }

void SCharacterComponent::OnFrozen(bool bFrozen)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		p->Write<bool>(bFrozen);
		ent.SendNetEvent(m_netEvSetFrozen, p, pragma::networking::Protocol::SlowReliable);
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
		entThis.SendNetEvent(m_netEvSetAmmoCount, p, pragma::networking::Protocol::SlowReliable);
	}
}

void SCharacterComponent::SetActiveWeapon(BaseEntity *ent)
{
	BaseCharacterComponent::SetActiveWeapon(ent);
	auto &entThis = static_cast<SBaseEntity &>(GetEntity());
	if(entThis.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, ent);
		entThis.SendNetEvent(m_netEvSetActiveWeapon, p, pragma::networking::Protocol::SlowReliable);
	}
}

void SCharacterComponent::DropWeapon(std::string className)
{
	for(unsigned int i = 0; i < m_weapons.size(); i++) {
		EntityHandle &hEnt = m_weapons[i];
		if(hEnt.valid()) {
			BaseEntity *ent = hEnt.get();
			if(ent->GetClass() == className)
				DropWeapon(ent);
		}
	}
}
void SCharacterComponent::DropWeapon(BaseEntity *ent)
{
	if(m_weapons.empty())
		return;
	auto it = std::find_if(m_weapons.begin(), m_weapons.end(), [ent](EntityHandle &hEnt) { return (hEnt.valid() && hEnt.get() == ent) ? true : false; });
	if(it == m_weapons.end())
		return;
	auto *wep = it->get();
	m_weapons.erase(it);
	if(wep->IsWeapon())
		static_cast<pragma::SWeaponComponent *>(wep->GetWeaponComponent().get())->Drop();
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
	static_cast<pragma::SWeaponComponent *>(wepComponent.get())->Drop();
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

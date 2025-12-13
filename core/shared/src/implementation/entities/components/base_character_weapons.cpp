// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_character;

using namespace pragma;

std::vector<EntityHandle> &BaseCharacterComponent::GetWeapons() { return m_weapons; }
void BaseCharacterComponent::GiveWeapon(ecs::BaseEntity &ent)
{
	if(!ent.IsWeapon())
		return;
	m_weapons.push_back(ent.GetHandle());
	auto &wepComponent = *ent.GetWeaponComponent();
	auto *ownerComponent = wepComponent.GetOwnerComponent();
	if(ownerComponent != nullptr)
		ownerComponent->SetOwner(GetEntity());
	if(GetActiveWeapon() == nullptr)
		DeployWeapon(ent);
}
ecs::BaseEntity *BaseCharacterComponent::GiveWeapon(std::string className)
{
	auto &ent = GetEntity();
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	auto *wep = game->CreateEntity(className);
	if(wep == nullptr)
		return nullptr;
	if(!wep->IsWeapon()) {
		wep->Remove();
		return nullptr;
	}
	wep->Spawn();
	GiveWeapon(*wep);
	return wep;
}
ecs::BaseEntity *BaseCharacterComponent::GetActiveWeapon()
{
	if(m_weaponActive.valid() == false)
		return nullptr;
	return m_weaponActive.get();
}
ecs::BaseEntity *BaseCharacterComponent::GetWeapon(std::string className)
{
	string::to_lower(className);
	auto it = std::find_if(m_weapons.begin(), m_weapons.end(), [&className](const EntityHandle &hEnt) { return (hEnt.valid() && hEnt.get()->GetClass() == className) ? true : false; });
	return (it != m_weapons.end()) ? it->get() : nullptr;
}
std::vector<ecs::BaseEntity *> BaseCharacterComponent::GetWeapons(std::string className)
{
	std::vector<ecs::BaseEntity *> weapons;
	GetWeapons(className, weapons);
	return weapons;
}
void BaseCharacterComponent::GetWeapons(std::string className, std::vector<ecs::BaseEntity *> &weapons)
{
	string::to_lower(className);
	for(auto &hEnt : m_weapons) {
		if(hEnt.valid() && hEnt->GetClass() == className)
			weapons.push_back(hEnt.get());
	}
}
void BaseCharacterComponent::DeployWeapon(const std::string &className)
{
	if(m_weapons.empty())
		return;
	auto it = std::find_if(m_weapons.begin(), m_weapons.end(), [&className](EntityHandle &hEnt) { return (hEnt.valid() && hEnt->GetClass() == className) ? true : false; });
	if(it == m_weapons.end())
		return;
	DeployWeapon(*it->get());
}
void BaseCharacterComponent::DeployWeapon(ecs::BaseEntity &ent)
{
	if(&ent == GetActiveWeapon())
		return;
	HolsterWeapon();
	if(m_weapons.empty())
		return;
	auto it = std::find_if(m_weapons.begin(), m_weapons.end(), [&ent](const EntityHandle &hOther) { return &ent == hOther.get(); });
	if(it == m_weapons.end() || it->get()->IsWeapon() == false)
		return;
	it->get()->GetWeaponComponent()->Deploy();
	SetActiveWeapon(it->get());

	auto evOnDeployWeapon = CEOnDeployWeapon {ent};
	BroadcastEvent(baseCharacterComponent::EVENT_ON_DEPLOY_WEAPON, evOnDeployWeapon);
}
void BaseCharacterComponent::SetActiveWeapon(ecs::BaseEntity *ent)
{
	if(ent == nullptr)
		m_weaponActive = EntityHandle();
	else
		m_weaponActive = ent->GetHandle();

	auto evOnSetActiveWeapon = CEOnSetActiveWeapon {ent};
	BroadcastEvent(baseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON, evOnSetActiveWeapon);
}
void BaseCharacterComponent::HolsterWeapon()
{
	if(m_weaponActive.valid() && m_weaponActive->IsWeapon())
		m_weaponActive->GetWeaponComponent()->Holster();
	m_weaponActive = EntityHandle();
}

void BaseCharacterComponent::RemoveWeapon(std::string className)
{
	if(m_weapons.empty())
		return;
	for(auto it = m_weapons.begin(); it != m_weapons.end();) {
		auto &hEnt = *it;
		if(hEnt.valid() == false) {
			it = m_weapons.erase(it);
			continue;
		}
		if(string::compare(className.c_str(), hEnt->GetClass().c_str(), false)) {
			it = RemoveWeapon(*it->get());
			continue;
		}
	}
}
std::vector<EntityHandle>::iterator BaseCharacterComponent::RemoveWeapon(ecs::BaseEntity &ent)
{
	auto it = std::find_if(m_weapons.begin(), m_weapons.end(), [&ent](const EntityHandle &hEnt) { return (hEnt.get() == &ent) ? true : false; });
	if(it == m_weapons.end())
		return it;
	ent.Remove();
	return m_weapons.erase(it);
}
void BaseCharacterComponent::RemoveWeapons()
{
	for(auto &hEnt : m_weapons) {
		if(hEnt.valid() == false)
			continue;
		hEnt->Remove();
	}
	m_weapons.clear();
}
bool BaseCharacterComponent::HasWeapon(std::string className)
{
	string::to_lower(className);
	auto it = std::find_if(m_weapons.begin(), m_weapons.end(), [&className](const EntityHandle &hEnt) { return (hEnt.valid() && hEnt.get()->GetClass() == className) ? true : false; });
	return (it != m_weapons.end()) ? true : false;
}
void BaseCharacterComponent::PrimaryAttack()
{
	auto *wep = GetActiveWeapon();
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->PrimaryAttack();
}

void BaseCharacterComponent::SecondaryAttack()
{
	auto *wep = GetActiveWeapon();
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->SecondaryAttack();
}

void BaseCharacterComponent::TertiaryAttack()
{
	auto *wep = GetActiveWeapon();
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->TertiaryAttack();
}

void BaseCharacterComponent::Attack4()
{
	auto *wep = GetActiveWeapon();
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->Attack4();
}

void BaseCharacterComponent::ReloadWeapon()
{
	auto *wep = GetActiveWeapon();
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->Reload();
}

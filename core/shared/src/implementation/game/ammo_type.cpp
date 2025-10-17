// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "algorithm"


module pragma.shared;

import :game.ammo_type;

AmmoType::AmmoType(UInt32 _id, const std::string &_name, DAMAGETYPE _dmgType, Int32 _dmg, Float _force) : id(_id), name(_name), damageType(_dmgType), damage(_dmg), force(_force) {}

AmmoTypeManager::AmmoTypeManager() {}

Bool AmmoTypeManager::RegisterAmmoType(const std::string &name, Int32 damage, Float force, DAMAGETYPE dmgType, AmmoType **ammoOut)
{
	auto *ammoType = GetAmmoType(name);
	if(ammoType != nullptr) {
		if(ammoOut != nullptr)
			*ammoOut = ammoType;
		return false;
	}
	m_ammoTypes.push_back(std::unique_ptr<AmmoType>(new AmmoType(CUInt32(m_ammoTypes.size() + 1), name, dmgType, damage, force)));
	if(ammoOut != nullptr)
		*ammoOut = m_ammoTypes.back().get();
	return true;
}
AmmoType *AmmoTypeManager::GetAmmoType(const std::string &name, UInt32 *ammoId)
{
	auto it = std::find_if(m_ammoTypes.begin(), m_ammoTypes.end(), [&name](std::unique_ptr<AmmoType> &ammoType) { return (name == ammoType->name) ? true : false; });
	if(it == m_ammoTypes.end()) {
		if(ammoId != nullptr)
			*ammoId = 0;
		return nullptr;
	}
	if(ammoId != nullptr)
		*ammoId = (*it)->id;
	return it->get();
}
AmmoType *AmmoTypeManager::GetAmmoType(UInt32 ammoId)
{
	if(ammoId == 0)
		return nullptr;
	--ammoId;
	if(ammoId >= m_ammoTypes.size())
		return nullptr;
	return m_ammoTypes[ammoId].get();
}

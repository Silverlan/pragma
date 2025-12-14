// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.game;

pragma::game::AmmoTypeManager &pragma::Game::GetAmmoTypeManager() { return *m_ammoTypes.get(); }
Bool pragma::Game::RegisterAmmoType(const std::string &name, Int32 damage, Float force, DamageType dmgType, game::AmmoType **ammoOut) { return m_ammoTypes->RegisterAmmoType(name, damage, force, dmgType, ammoOut); }
pragma::game::AmmoType *pragma::Game::GetAmmoType(const std::string &name, UInt32 *ammoId) { return m_ammoTypes->GetAmmoType(name, ammoId); }
pragma::game::AmmoType *pragma::Game::GetAmmoType(UInt32 ammoId) { return m_ammoTypes->GetAmmoType(ammoId); }

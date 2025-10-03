// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/umath.h"

module pragma.shared;

import :game.game;

AmmoTypeManager &Game::GetAmmoTypeManager() { return *m_ammoTypes.get(); }
Bool Game::RegisterAmmoType(const std::string &name, Int32 damage, Float force, DAMAGETYPE dmgType, AmmoType **ammoOut) { return m_ammoTypes->RegisterAmmoType(name, damage, force, dmgType, ammoOut); }
AmmoType *Game::GetAmmoType(const std::string &name, UInt32 *ammoId) { return m_ammoTypes->GetAmmoType(name, ammoId); }
AmmoType *Game::GetAmmoType(UInt32 ammoId) { return m_ammoTypes->GetAmmoType(ammoId); }

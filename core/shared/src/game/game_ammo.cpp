/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/util/ammo_type.h"

AmmoTypeManager &Game::GetAmmoTypeManager() { return *m_ammoTypes.get(); }
Bool Game::RegisterAmmoType(const std::string &name, Int32 damage, Float force, DAMAGETYPE dmgType, AmmoType **ammoOut) { return m_ammoTypes->RegisterAmmoType(name, damage, force, dmgType, ammoOut); }
AmmoType *Game::GetAmmoType(const std::string &name, UInt32 *ammoId) { return m_ammoTypes->GetAmmoType(name, ammoId); }
AmmoType *Game::GetAmmoType(UInt32 ammoId) { return m_ammoTypes->GetAmmoType(ammoId); }

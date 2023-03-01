/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __AMMO_TYPE_H__
#define __AMMO_TYPE_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>
#include "pragma/game/damagetype.h"
#include "pragma/game/damageinfo.h"
#include <vector>
#include <memory>

class AmmoTypeManager;
struct DLLNETWORK AmmoType {
  public:
	friend AmmoTypeManager;
  protected:
	AmmoType(UInt32 id, const std::string &name, DAMAGETYPE dmgType, Int32 dmg, Float force);
  public:
	std::string name;
	UInt32 id;
	DAMAGETYPE damageType;
	Int32 damage;
	Float force;
};

class DLLNETWORK AmmoTypeManager {
  private:
	std::vector<std::unique_ptr<AmmoType>> m_ammoTypes;
  public:
	AmmoTypeManager();
	Bool RegisterAmmoType(const std::string &name, Int32 damage = 10, Float force = 200.f, DAMAGETYPE dmgType = DAMAGETYPE::BULLET, AmmoType **ammoOut = nullptr);
	AmmoType *GetAmmoType(const std::string &name, UInt32 *ammoId = nullptr);
	AmmoType *GetAmmoType(UInt32 ammoId);
	AmmoTypeManager(AmmoTypeManager &) = delete;
	AmmoTypeManager &operator=(const AmmoTypeManager &other) = delete;
};

#endif

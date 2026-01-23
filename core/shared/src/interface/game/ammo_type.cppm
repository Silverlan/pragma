// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:game.ammo_type;

import :game.enums;

export namespace pragma::game {
	class AmmoTypeManager;
	struct DLLNETWORK AmmoType {
	  public:
		friend AmmoTypeManager;
	  protected:
		AmmoType(UInt32 id, const std::string &name, DamageType dmgType, Int32 dmg, Float force);
	  public:
		std::string name;
		UInt32 id;
		DamageType damageType;
		Int32 damage;
		Float force;
	};

	class DLLNETWORK AmmoTypeManager {
	  private:
		std::vector<std::unique_ptr<AmmoType>> m_ammoTypes;
	  public:
		AmmoTypeManager();
		Bool RegisterAmmoType(const std::string &name, Int32 damage = 10, Float force = 200.f, DamageType dmgType = Bullet, AmmoType **ammoOut = nullptr);
		AmmoType *GetAmmoType(const std::string &name, UInt32 *ammoId = nullptr);
		AmmoType *GetAmmoType(UInt32 ammoId);
		AmmoTypeManager(AmmoTypeManager &) = delete;
		AmmoTypeManager &operator=(const AmmoTypeManager &other) = delete;
	};
};

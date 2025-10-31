// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:game.ammo_type;

import :game.enums;

export {
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
};

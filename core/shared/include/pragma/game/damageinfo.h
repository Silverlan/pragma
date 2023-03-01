/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __DAMAGEINFO_H__
#define __DAMAGEINFO_H__

#include "pragma/networkdefinitions.h"
#include "pragma/game/damagetype.h"
#include "pragma/types.hpp"
#include <mathutil/glmutil.h>
#include "pragma/physics/hitboxes.h"
#include "pragma/entities/baseentity_handle.h"

class BaseEntity;
class DLLNETWORK DamageInfo {
  protected:
	unsigned short m_damage = 0u;
	unsigned int m_types = 0u;
	Vector3 m_source = {};
	Vector3 m_force = {};
	Vector3 m_hitPosition = {};
	EntityHandle m_attacker = {};
	EntityHandle m_inflictor = {};
	HitGroup m_hitGroup = HitGroup::Generic;
  public:
	DamageInfo() = default;
	~DamageInfo() = default;
	void SetDamage(unsigned short dmg);
	void AddDamage(unsigned short dmg);
	void ScaleDamage(float scale);
	unsigned short GetDamage();
	BaseEntity *GetAttacker();
	void SetAttacker(const BaseEntity *ent);
	void SetAttacker(const EntityHandle &hnd);
	BaseEntity *GetInflictor();
	void SetInflictor(const BaseEntity *ent);
	void SetInflictor(const EntityHandle &hnd);
	unsigned int GetDamageTypes();
	void SetDamageType(DAMAGETYPE type);
	void AddDamageType(DAMAGETYPE type);
	void RemoveDamageType(DAMAGETYPE type);
	bool IsDamageType(DAMAGETYPE type);
	void SetSource(const Vector3 &origin);
	Vector3 &GetSource();
	void SetHitPosition(const Vector3 &pos);
	Vector3 &GetHitPosition();
	void SetForce(const Vector3 &force);
	Vector3 &GetForce();
	HitGroup GetHitGroup() const;
	void SetHitGroup(HitGroup hitGroup);
};

DLLNETWORK std::ostream &operator<<(std::ostream &out, const DamageInfo &o);

#endif

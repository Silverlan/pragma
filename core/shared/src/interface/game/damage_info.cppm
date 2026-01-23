// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:game.damage_info;

import :entities.base_entity_handle;
import :game.enums;
import :physics.hitbox;

export namespace pragma::game {
	class DLLNETWORK DamageInfo {
	  protected:
		unsigned short m_damage = 0u;
		unsigned int m_types = 0u;
		Vector3 m_source = {};
		Vector3 m_force = {};
		Vector3 m_hitPosition = {};
		EntityHandle m_attacker = {};
		EntityHandle m_inflictor = {};
		physics::HitGroup m_hitGroup = physics::HitGroup::Generic;
	  public:
		DamageInfo() = default;
		~DamageInfo() = default;
		void SetDamage(unsigned short dmg);
		void AddDamage(unsigned short dmg);
		void ScaleDamage(float scale);
		unsigned short GetDamage();
		ecs::BaseEntity *GetAttacker();
		void SetAttacker(const ecs::BaseEntity *ent);
		void SetAttacker(const EntityHandle &hnd);
		ecs::BaseEntity *GetInflictor();
		void SetInflictor(const ecs::BaseEntity *ent);
		void SetInflictor(const EntityHandle &hnd);
		unsigned int GetDamageTypes();
		void SetDamageType(DamageType type);
		void AddDamageType(DamageType type);
		void RemoveDamageType(DamageType type);
		bool IsDamageType(DamageType type);
		void SetSource(const Vector3 &origin);
		Vector3 &GetSource();
		void SetHitPosition(const Vector3 &pos);
		Vector3 &GetHitPosition();
		void SetForce(const Vector3 &force);
		Vector3 &GetForce();
		physics::HitGroup GetHitGroup() const;
		void SetHitGroup(physics::HitGroup hitGroup);
	};
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const DamageInfo &o);
};

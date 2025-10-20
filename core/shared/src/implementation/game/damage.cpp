// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <memory>

#include "memory"



module pragma.shared;

import :game.game;

void pragma::Game::SplashDamage(const Vector3 &origin, Float radius, DamageInfo &dmg, const std::function<bool(pragma::ecs::BaseEntity *, DamageInfo &)> &callback)
{
	auto &force = dmg.GetForce();
	auto forceLen = uvec::length(force);
	auto damage = dmg.GetDamage();
	if(radius <= 0.f)
		return;
	struct EntityCandidate {
		EntityCandidate(pragma::ecs::BaseEntity *ent, Vector3 &pos, Float dist) : hEntity(ent->GetHandle()), position(pos), distance(dist) {}
		EntityHandle hEntity;
		Vector3 position;
		Float distance;
	};
	std::vector<EntityCandidate> ents;
	pragma::ecs::EntityIterator entIt {*this};
	entIt.AttachFilter<EntityIteratorFilterSphere>(origin, radius);
	for(auto *ent : entIt) {
		auto pTrComponent = ent->GetTransformComponent();
		auto pPhysComponent = ent->GetPhysicsComponent();
		Vector3 min {};
		Vector3 max {};
		if(pPhysComponent != nullptr)
			pPhysComponent->GetCollisionBounds(&min, &max);
		Vector3 pos;
		umath::geometry::closest_point_on_aabb_to_point(min, max, origin - pTrComponent->GetPosition(), &pos);
		auto dist = uvec::length(pos);
		ents.push_back(EntityCandidate(ent, pos, dist));
	}
	std::vector<EntityHandle> entsFilter;
	entsFilter.reserve(2);
	auto *attacker = dmg.GetAttacker();
	if(attacker != nullptr)
		entsFilter.push_back(attacker->GetHandle());
	auto *inflictor = dmg.GetInflictor();
	if(inflictor != nullptr)
		entsFilter.push_back(inflictor->GetHandle());
	auto traceFilter = std::make_shared<::pragma::physics::MultiEntityRayCastFilterCallback>(std::move(entsFilter));

	auto *entOrigin = (attacker != nullptr) ? attacker : inflictor;
	for(auto it = ents.begin(); it != ents.end(); ++it) {
		auto &c = *it;
		if(c.hEntity.valid()) {
			auto *ent = c.hEntity.get();
			auto pTrComponent = ent->GetTransformComponent();
			if(pTrComponent == nullptr)
				continue;
			auto pos = c.position + pTrComponent->GetPosition();

			auto dir = pos - origin;
			auto l = uvec::length(dir);
			auto pPhysComponent = ent->GetPhysicsComponent();
			if(l == 0.f) // Damage origin is within entity
			{
				pos = pTrComponent->GetPosition();
				if(pPhysComponent != nullptr)
					pos += pPhysComponent->GetCollisionCenter();
				dir = pos - origin;
				l = uvec::length(dir);
				if(l == 0.f) // Damage origin is at entity center
				{
					pos = pTrComponent->GetPosition();
					dir = pos - origin;
					l = uvec::length(dir);
					if(l == 0.f)                                 // Damage origin is at entity origin
						dir = uvec::create_random_unit_vector(); // Just use a random direction
				}
			}
			if(l != 0.f)
				dir /= l;

			TraceData data;
			data.SetSource(origin);
			data.SetTarget(pos);
			data.SetFilter(traceFilter);
			data.SetFlags(pragma::physics::RayCastFlags::Default | pragma::physics::RayCastFlags::InvertFilter);
			if(entOrigin != nullptr && pPhysComponent != nullptr) {
				data.SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter());
				data.SetCollisionFilterMask(pPhysComponent->GetCollisionFilterMask());
			}
			auto r = RayCast(data);
			if(r.hitType != pragma::physics::RayCastHitType::None && r.entity.get() != ent) {
				auto pPhysComponent = ent->GetPhysicsComponent();
				auto &pos = pTrComponent->GetPosition();
				auto center = (pPhysComponent != nullptr) ? pPhysComponent->GetCollisionCenter() : Vector3 {};
				Vector3 min {};
				Vector3 max {};
				if(pPhysComponent != nullptr)
					pPhysComponent->GetCollisionBounds(&min, &max);
				min = pos + center + (min - center) * 0.5f;
				data.SetTarget(min);
				r = RayCast(data);
				if(r.hitType != pragma::physics::RayCastHitType::None && r.entity.get() != ent) {
					max = pos + center + (max - center) * 0.5f;
					data.SetTarget(max);
					r = RayCast(data);
				}
			}
			if(r.hitType == pragma::physics::RayCastHitType::None || r.entity.get() == ent) {
				auto pDamageableComponent = ent->GetComponent<pragma::DamageableComponent>();
				if(pDamageableComponent.valid()) {
					auto scale = (radius - c.distance) / radius;
					DamageInfo dmgEnt;
					dmgEnt.SetAttacker(dmg.GetAttacker());
					dmgEnt.SetInflictor(dmg.GetInflictor());
					dmgEnt.SetDamage(CUInt16(CFloat(damage) * scale));
					dmgEnt.SetSource(const_cast<Vector3 &>(origin));
					dmgEnt.SetDamageType(static_cast<DAMAGETYPE>(dmg.GetDamageTypes()));
					dmgEnt.SetForce(dir * (forceLen * scale));
					if(callback == nullptr || callback(ent, dmgEnt) == true)
						pDamageableComponent->TakeDamage(dmgEnt);
				}
			}
		}
	}
}
void pragma::Game::SplashDamage(const Vector3 &origin, Float radius, UInt32 damage, Float force, pragma::ecs::BaseEntity *attacker, pragma::ecs::BaseEntity *inflictor, const std::function<bool(pragma::ecs::BaseEntity *, DamageInfo &)> &callback)
{
	DamageInfo info;
	info.SetForce(Vector3(force, 0.f, 0.f));
	info.SetAttacker(attacker);
	info.SetInflictor(inflictor);
	info.SetDamage(CUInt16(damage));
	info.SetDamageType(DAMAGETYPE::EXPLOSION);
	SplashDamage(origin, radius, info, callback);
}
void pragma::Game::SplashDamage(const Vector3 &origin, Float radius, UInt32 damage, Float force, const EntityHandle &attacker, const EntityHandle &inflictor, const std::function<bool(pragma::ecs::BaseEntity *, DamageInfo &)> &callback)
{
	SplashDamage(origin, radius, damage, force, const_cast<pragma::ecs::BaseEntity *>(attacker.get()), const_cast<pragma::ecs::BaseEntity *>(inflictor.get()), callback);
}

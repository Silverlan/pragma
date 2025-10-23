// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <algorithm>
#include <string>

module pragma.shared;

import :entities.components.environment.effects.base_explosion;

using namespace pragma;

void BaseEnvExplosionComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "explode", false))
			Explode();
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	GetEntity().AddComponent("io");
}

void BaseEnvExplosionComponent::Explode()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto &origin = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	float radius = 512.f;
	unsigned short damage = 12;
	Vector3 force(0.f, 0.f, 0.f);
	pragma::ecs::EntityIterator entIt {*ent.GetNetworkState()->GetGameState()};
	entIt.AttachFilter<EntityIteratorFilterComponent>("transform");
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::DamageableComponent>>();
	for(auto *entOther : entIt) {
		auto pTrComponentOther = entOther->GetTransformComponent();
		auto pDamageableComponentOther = entOther->GetComponent<pragma::DamageableComponent>();
		auto pPhysComponentOther = entOther->GetPhysicsComponent();
		auto &pos = pTrComponentOther->GetPosition();
		Vector3 min {};
		Vector3 max {};
		if(pPhysComponentOther)
			pPhysComponentOther->GetCollisionBounds(&min, &max);
		Vector3 r;
		umath::geometry::closest_point_on_aabb_to_point((min + pos), (max + pos), origin, &r);
		float d = glm::distance(origin, r);
		if(d <= radius) {
			// TODO: Raytrace
			DamageInfo dmg;
			dmg.SetAttacker(&ent);
			dmg.SetDamage(damage);
			dmg.SetDamageType(DAMAGETYPE::EXPLOSION);
			dmg.SetForce(force);
			dmg.SetHitPosition(r);
			dmg.SetInflictor(&ent);
			dmg.SetSource(origin);
			pDamageableComponentOther->TakeDamage(dmg);
		}
	}
}

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.effects.base_explosion;

using namespace pragma;

void BaseEnvExplosionComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "explode", false))
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
	ecs::EntityIterator entIt {*ent.GetNetworkState()->GetGameState()};
	entIt.AttachFilter<EntityIteratorFilterComponent>("transform");
	entIt.AttachFilter<TEntityIteratorFilterComponent<DamageableComponent>>();
	for(auto *entOther : entIt) {
		auto pTrComponentOther = entOther->GetTransformComponent();
		auto pDamageableComponentOther = entOther->GetComponent<DamageableComponent>();
		auto pPhysComponentOther = entOther->GetPhysicsComponent();
		auto &pos = pTrComponentOther->GetPosition();
		Vector3 min {};
		Vector3 max {};
		if(pPhysComponentOther)
			pPhysComponentOther->GetCollisionBounds(&min, &max);
		Vector3 r;
		math::geometry::closest_point_on_aabb_to_point((min + pos), (max + pos), origin, &r);
		float d = glm::distance(origin, r);
		if(d <= radius) {
			// TODO: Raytrace
			game::DamageInfo dmg;
			dmg.SetAttacker(&ent);
			dmg.SetDamage(damage);
			dmg.SetDamageType(Explosion);
			dmg.SetForce(force);
			dmg.SetHitPosition(r);
			dmg.SetInflictor(&ent);
			dmg.SetSource(origin);
			pDamageableComponentOther->TakeDamage(dmg);
		}
	}
}

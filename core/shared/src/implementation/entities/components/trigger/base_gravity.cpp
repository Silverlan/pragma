// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.triggers.base_gravity;

using namespace pragma;

void Entity::TriggerGravity::apply_gravity(ecs::BaseEntity *ent, uint32_t flags, const Vector3 &gravityDir, const Vector3 &dirUp, bool bUseForce, float gravityForce, std::shared_ptr<Vector3> *upDir)
{
	if(flags & math::to_integral(SpawnFlags::ChangeOrientation)) {
		auto charComponent = ent->GetCharacterComponent();
		if(charComponent.valid()) {
			if(upDir != nullptr) {
				auto *orientC = charComponent->GetOrientationComponent();
				*upDir = std::make_unique<Vector3>(orientC ? orientC->GetUpDirection() : uvec::PRM_UP);
			}
			charComponent->SetCharacterOrientation(dirUp);
		}
	}
	auto gravityComponent = ent->GetComponent<GravityComponent>();
	if(gravityComponent.expired())
		return;
	if(flags & math::to_integral(SpawnFlags::AffectDirection))
		gravityComponent->SetGravityOverride(gravityDir);
	if(bUseForce == true)
		gravityComponent->SetGravityOverride(gravityForce);
	else
		gravityComponent->SetGravityScale(gravityForce);
}

void BaseEntityTriggerGravityComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("touch");
}
void BaseEntityTriggerGravityComponent::OnStartTouch(ecs::BaseEntity *ent)
{
	auto &settings = m_gravityReset.insert(std::make_pair(pragma::util::make_shared<EntityHandle>(ent->GetHandle()), GravitySettings {})).first->second;
	auto pEntGravityComponent = ent->GetComponent<GravityComponent>();
	settings.scale = pEntGravityComponent.valid() ? pEntGravityComponent->GetGravityScale() : 1.f;
	if(pEntGravityComponent.valid()) {
		if(pEntGravityComponent->HasGravityForceOverride())
			settings.force = pragma::util::make_shared<float>(pEntGravityComponent->GetGravity());
		if(pEntGravityComponent->HasGravityDirectionOverride())
			settings.dir = pragma::util::make_shared<Vector3>(pEntGravityComponent->GetGravityDirection());
	}

	auto &entThis = GetEntity();
	Entity::TriggerGravity::apply_gravity(ent, entThis.GetSpawnFlags(), -m_kvGravityDir, m_kvGravityDir, m_kvUseForce, m_kvGravityForce, &settings.dirMove);
}
void BaseEntityTriggerGravityComponent::OnEndTouch(ecs::BaseEntity *ent)
{
	auto &entThis = GetEntity();
	for(auto it = m_gravityReset.begin(); it != m_gravityReset.end();) {
		auto &hEnt = *it->first;
		if(hEnt.valid() && hEnt.get() == ent) {
			auto &settings = it->second;
			if(entThis.GetSpawnFlags() & math::to_integral(Entity::TriggerGravity::SpawnFlags::ResetOnEndTouch)) {
				auto pEntGravityComponent = ent->GetComponent<GravityComponent>();
				if(pEntGravityComponent.valid())
					pEntGravityComponent->SetGravityScale(settings.scale);
				Entity::TriggerGravity::apply_gravity(ent, entThis.GetSpawnFlags(), (settings.dir != nullptr) ? *settings.dir : Vector3 {}, (settings.dirMove != nullptr) ? *settings.dirMove : Vector3 {}, m_kvUseForce, (settings.force != nullptr) ? *settings.force : 0.f);

				OnResetGravity(ent, settings);
			}
			m_gravityReset.erase(it);
			break;
		}
	}
}
void BaseEntityTriggerGravityComponent::OnResetGravity(ecs::BaseEntity *ent, GravitySettings &settings) {}
util::EventReply BaseEntityTriggerGravityComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseTouchComponent::EVENT_ON_START_TOUCH) {
		auto &touchData = static_cast<const CETouchData &>(evData);
		OnStartTouch(touchData.entity);
	}
	if(eventId == baseTouchComponent::EVENT_ON_END_TOUCH) {
		auto &touchData = static_cast<const CETouchData &>(evData);
		OnEndTouch(touchData.entity);
	}
	return util::EventReply::Unhandled;
}

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.triggers.base_push;

using namespace pragma;

void BaseTriggerPushComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "push_dir", false)) {
			EulerAngles ang(kvData.value);
			m_kvPushDir = ang.Forward();
		}
		else if(pragma::string::compare<std::string>(kvData.key, "push_speed", false))
			m_kvPushSpeed = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "change_duration", false))
			m_kvChangeDuration = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("model");
	ent.AddComponent("touch");
	ent.AddComponent("toggle");
}

void BaseTriggerPushComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pToggleComponent = dynamic_cast<BaseToggleComponent *>(&component);
	if(pToggleComponent != nullptr)
		m_toggleComponent = pToggleComponent->GetHandle<BaseToggleComponent>();
}

void BaseTriggerPushComponent::OnTick(double dt)
{
	if(m_toggleComponent.valid() && m_toggleComponent->IsTurnedOn() == false)
		return;
	for(auto it = m_entityPushQueue.begin(); it != m_entityPushQueue.end();) {
		auto &info = *it;
		if(info.hEntity.valid() == false) {
			it = m_entityPushQueue.erase(it);
			continue;
		}
		auto *ent = info.hEntity.get();
		auto t = math::min(info.t, static_cast<float>(dt) / m_kvChangeDuration);
		auto pVelComponentEnt = ent->GetComponent<VelocityComponent>();
		auto vel = pVelComponentEnt.valid() ? pVelComponentEnt->GetVelocity() : Vector3 {};
		vel = glm::gtc::slerp(uvec::get_normal(vel), m_kvPushDir, t) * uvec::length(vel);
		//uvec::rotate(&vel,info.rotation *t);
		if(pVelComponentEnt.valid())
			pVelComponentEnt->SetVelocity(vel);
		if((info.t -= t) <= 0.f) {
			it = m_entityPushQueue.erase(it);
			continue;
		}
		++it;
		/*auto speed = uvec::length(vel);
		auto speedChange = pragma::math::min(speed,pragma::math::min(info.speed,static_cast<float>(tDelta) *(1.f /m_kvChangeDuration) *info.initialSpeed));
		if(speedChange <= 0.f)
		{
			it = m_entityPushQueue.erase(it);
			continue;
		}
		info.speed -= speedChange;
		if(speed > 0.f)
			vel /= speed;
		vel *= pragma::math::max(speed -speedChange,0.f);
		vel += m_kvPushDir *speedChange;
		ent->SetVelocity(vel);
		++it;*/
	}
	if(m_kvPushSpeed == 0.f)
		return;
	Vector3 vel = m_kvPushDir * m_kvPushSpeed;
	vel *= dt;
	auto *pTouchComponent = static_cast<BaseTouchComponent *>(GetEntity().FindComponent("touch").get());
	for(auto &touch : pTouchComponent->GetTouchingInfo()) {
		auto &hEnt = touch.touch.entity;
		if(hEnt.valid() == false || touch.triggered == false)
			continue;
		auto pVelComponentEnt = hEnt.get()->GetComponent<VelocityComponent>();
		if(pVelComponentEnt.valid())
			pVelComponentEnt->AddVelocity(vel);
	}
}

util::EventReply BaseTriggerPushComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseTouchComponent::EVENT_ON_START_TOUCH) {
		auto &entThis = GetEntity();
		if(entThis.GetSpawnFlags() & math::to_integral(SpawnFlags::ChangeVelocityDirection)) {
			auto &touchData = static_cast<const CETouchData &>(evData);
			m_entityPushQueue.push_back({});
			auto &info = m_entityPushQueue.back();
			info.hEntity = touchData.entity->GetHandle();
			auto pVelComponentEnt = touchData.entity->GetComponent<VelocityComponent>();
			info.rotation = uvec::get_rotation(pVelComponentEnt.valid() ? uvec::get_normal(pVelComponentEnt->GetVelocity()) : uvec::PRM_UP, m_kvPushDir);
		}
	}
	return util::EventReply::Unhandled;
}

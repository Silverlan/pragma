/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/basetriggergravity.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/orientation_component.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"

using namespace pragma;

void pragma::Entity::TriggerGravity::apply_gravity(BaseEntity *ent, uint32_t flags, const Vector3 &gravityDir, const Vector3 &dirUp, bool bUseForce, float gravityForce, std::shared_ptr<Vector3> *upDir)
{
	if(flags & umath::to_integral(SpawnFlags::ChangeOrientation)) {
		auto charComponent = ent->GetCharacterComponent();
		if(charComponent.valid()) {
			if(upDir != nullptr) {
				auto *orientC = charComponent->GetOrientationComponent();
				*upDir = std::make_unique<Vector3>(orientC ? orientC->GetUpDirection() : uvec::UP);
			}
			charComponent->SetCharacterOrientation(dirUp);
		}
	}
	auto gravityComponent = ent->GetComponent<pragma::GravityComponent>();
	if(gravityComponent.expired())
		return;
	if(flags & umath::to_integral(SpawnFlags::AffectDirection))
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
void BaseEntityTriggerGravityComponent::OnStartTouch(BaseEntity *ent)
{
	auto &settings = m_gravityReset.insert(std::make_pair(std::make_shared<EntityHandle>(ent->GetHandle()), GravitySettings {})).first->second;
	auto pEntGravityComponent = ent->GetComponent<pragma::GravityComponent>();
	settings.scale = pEntGravityComponent.valid() ? pEntGravityComponent->GetGravityScale() : 1.f;
	if(pEntGravityComponent.valid()) {
		if(pEntGravityComponent->HasGravityForceOverride())
			settings.force = std::make_shared<float>(pEntGravityComponent->GetGravity());
		if(pEntGravityComponent->HasGravityDirectionOverride())
			settings.dir = std::make_shared<Vector3>(pEntGravityComponent->GetGravityDirection());
	}

	auto &entThis = GetEntity();
	pragma::Entity::TriggerGravity::apply_gravity(ent, entThis.GetSpawnFlags(), -m_kvGravityDir, m_kvGravityDir, m_kvUseForce, m_kvGravityForce, &settings.dirMove);
}
void BaseEntityTriggerGravityComponent::OnEndTouch(BaseEntity *ent)
{
	auto &entThis = GetEntity();
	for(auto it = m_gravityReset.begin(); it != m_gravityReset.end();) {
		auto &hEnt = *it->first;
		if(hEnt.valid() && hEnt.get() == ent) {
			auto &settings = it->second;
			if(entThis.GetSpawnFlags() & umath::to_integral(pragma::Entity::TriggerGravity::SpawnFlags::ResetOnEndTouch)) {
				auto pEntGravityComponent = ent->GetComponent<pragma::GravityComponent>();
				if(pEntGravityComponent.valid())
					pEntGravityComponent->SetGravityScale(settings.scale);
				pragma::Entity::TriggerGravity::apply_gravity(ent, entThis.GetSpawnFlags(), (settings.dir != nullptr) ? *settings.dir : Vector3 {}, (settings.dirMove != nullptr) ? *settings.dirMove : Vector3 {}, m_kvUseForce, (settings.force != nullptr) ? *settings.force : 0.f);

				OnResetGravity(ent, settings);
			}
			m_gravityReset.erase(it);
			break;
		}
	}
}
void BaseEntityTriggerGravityComponent::OnResetGravity(BaseEntity *ent, GravitySettings &settings) {}
util::EventReply BaseEntityTriggerGravityComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseTouchComponent::EVENT_ON_START_TOUCH) {
		auto &touchData = static_cast<const pragma::CETouchData &>(evData);
		OnStartTouch(touchData.entity);
	}
	if(eventId == BaseTouchComponent::EVENT_ON_END_TOUCH) {
		auto &touchData = static_cast<const pragma::CETouchData &>(evData);
		OnEndTouch(touchData.entity);
	}
	return util::EventReply::Unhandled;
}

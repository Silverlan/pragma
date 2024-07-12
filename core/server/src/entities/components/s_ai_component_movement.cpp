/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/components/movement_component.hpp>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

decltype(SAIComponent::s_plDebugAiNav) SAIComponent::s_plDebugAiNav {};

bool SAIComponent::IsMoving() const { return m_moveInfo.moving; }

void SAIComponent::OnPathNodeChanged(uint32_t nodeIdx)
{
	BaseAIComponent::OnPathNodeChanged(nodeIdx);
	for(auto &hPl : s_plDebugAiNav) {
		if(hPl.expired() == false)
			_debugSendNavInfo(*hPl.get());
	}
	CEOnPathNodeChanged evData {nodeIdx};
	BroadcastEvent(EVENT_ON_PATH_NODE_CHANGED, evData);
}

void SAIComponent::OnPathChanged()
{
	BaseAIComponent::OnPathChanged();
	for(auto &hPl : s_plDebugAiNav) {
		if(hPl.expired() == false)
			_debugSendNavInfo(*hPl.get());
	}
	BroadcastEvent(EVENT_ON_PATH_CHANGED);
}

void SAIComponent::OnPrePhysicsSimulate()
{
	auto &ent = GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	if(charComponent.valid()) {
		auto *movementC = charComponent->GetMovementComponent();
		if(movementC && movementC->CanMove() == false)
			return;
	}
	if(m_moveInfo.moving == false)
		return;
	pragma::SAIComponent::AIAnimationInfo info {};
	info.SetPlayAsSchedule(false);
	PlayActivity(m_moveInfo.moveActivity, info);

	auto pTrComponent = ent.GetTransformComponent();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(pTrComponent != nullptr && pVelComponent.valid()) {
		auto &vel = pVelComponent->GetVelocity();
		if(uvec::length_sqr(vel) > 1.f) {
			auto faceTarget = (m_moveInfo.faceTarget != nullptr) ? *m_moveInfo.faceTarget : (pTrComponent->GetPosition() + m_moveInfo.moveDir * 1024.f);
			if(uvec::cmp(faceTarget, uvec::ORIGIN) == false)
				TurnStep(faceTarget, m_moveInfo.turnSpeed.get());
		}
	}
}

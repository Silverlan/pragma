// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.ai;

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
	BroadcastEvent(sAIComponent::EVENT_ON_PATH_NODE_CHANGED, evData);
}

void SAIComponent::OnPathChanged()
{
	BaseAIComponent::OnPathChanged();
	for(auto &hPl : s_plDebugAiNav) {
		if(hPl.expired() == false)
			_debugSendNavInfo(*hPl.get());
	}
	BroadcastEvent(sAIComponent::EVENT_ON_PATH_CHANGED);
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
	AIAnimationInfo info {};
	info.SetPlayAsSchedule(false);
	PlayActivity(m_moveInfo.moveActivity, info);

	auto pTrComponent = ent.GetTransformComponent();
	auto pVelComponent = ent.GetComponent<VelocityComponent>();
	if(pTrComponent != nullptr && pVelComponent.valid()) {
		auto &vel = pVelComponent->GetVelocity();
		if(uvec::length_sqr(vel) > 1.f) {
			auto faceTarget = (m_moveInfo.faceTarget != nullptr) ? *m_moveInfo.faceTarget : (pTrComponent->GetPosition() + m_moveInfo.moveDir * 1024.f);
			if(uvec::cmp(faceTarget, uvec::PRM_ORIGIN) == false)
				TurnStep(faceTarget, m_moveInfo.turnSpeed.get());
		}
	}
}

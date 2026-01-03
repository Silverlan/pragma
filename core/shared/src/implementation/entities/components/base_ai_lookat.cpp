// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_ai;

using namespace pragma;

void BaseAIComponent::LookAtStep(float tDelta)
{
	// TODO: Implement forward / inverse kinematics and do this properly
	if((m_neckInfo.lookTargetType == LookTargetType::None && m_neckInfo.neckTurned == false))
		return;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(!pTrComponent)
		return;
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto t = game->CurTime();
	if(t > m_neckInfo.lookTime)
		ClearLookTarget();
	Vector3 pos = {};
	auto rot = uquat::identity();
	pos = pTrComponent->GetEyePosition();
	rot = pTrComponent->GetRotation();
	//if(ent.GetLocalBonePosition(m_neckInfo.boneId,pos,rot) == false)
	//	return;
	//ent.LocalToWorld(&pos,&rot);

	EulerAngles ang {};
	if(m_neckInfo.lookTargetType != LookTargetType::None) {
		auto tgtPos = GetLookTarget();
		auto dir = tgtPos - pos;
		uvec::normalize(&dir);

		auto rotInv = pTrComponent->GetRotation();
		uquat::inverse(rotInv);
		ang = EulerAngles(rotInv * uquat::create_look_rotation(dir, pTrComponent->GetUp()));
		// Deprecated
		// ang = uvec::to_angle(dir,m_entity->GetUp()) -m_entity->GetAngles();
	}
	const auto maxTurnSpeed = 320.f;
	const auto fadeTime = 0.1f;
	auto turnAcceleration = maxTurnSpeed / fadeTime; // Reach full acceleration after x seconds
	m_neckInfo.neckTurned = false;

	auto animComponent = ent.GetAnimatedComponent();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr || animComponent.expired())
		return;
	std::array<int32_t, 2> blendControllers = {0, 0};
	auto charComponent = ent.GetCharacterComponent();
	if(charComponent.valid())
		blendControllers = {charComponent->GetNeckPitchBlendController(), charComponent->GetNeckYawBlendController()};
	for(auto i = decltype(blendControllers.size()) {0}; i < blendControllers.size(); ++i) {
		auto val = animComponent->GetBlendController(blendControllers[i]);
		auto *blendController = hMdl->GetBlendController(blendControllers[i]);
		auto valTgt = ang[i];

		if(blendController != nullptr)
			valTgt = math::clamp_angle(valTgt, blendController->min, blendController->max);

		auto angDiff = math::get_angle_difference(val, valTgt);
		auto &turnSpeed = m_neckInfo.turnSpeed[i];
		if((math::abs(angDiff) / math::abs(turnSpeed)) < fadeTime) // We'll have reached the turn target soon, slow down (TODO: This might cause jittering)
			turnSpeed = math::approach(m_neckInfo.turnSpeed[i], 0.f, turnAcceleration * tDelta);
		else
			turnSpeed = math::approach(m_neckInfo.turnSpeed[i], maxTurnSpeed * static_cast<float>(math::sign(valTgt)), turnAcceleration * tDelta);
		val = math::approach_angle(val, valTgt, tDelta * math::abs(turnSpeed));
		animComponent->SetBlendController(blendControllers[i], val);
		if(val != 0.f)
			m_neckInfo.neckTurned = true;
	}
}
void BaseAIComponent::ClearLookTarget()
{
	if(m_neckInfo.lookTargetType == LookTargetType::None)
		return;
	m_neckInfo.lookTargetType = LookTargetType::None;
	m_neckInfo.lookTarget = {};
	m_neckInfo.hEntityLookTarget = EntityHandle {};
	m_neckInfo.lookTime = std::numeric_limits<float>::max();
	OnLookTargetChanged();
}
void BaseAIComponent::SetLookTarget(const Vector3 &pos, float t)
{
	m_neckInfo.lookTime = t;
	if(m_neckInfo.lookTargetType == LookTargetType::Position && uvec::cmp(pos, m_neckInfo.lookTarget) == true)
		return;
	m_neckInfo.lookTargetType = LookTargetType::Position;
	m_neckInfo.lookTarget = pos;
	m_neckInfo.hEntityLookTarget = EntityHandle {};

	OnLookTargetChanged();
}
void BaseAIComponent::SetLookTarget(const ecs::BaseEntity &ent, float t)
{
	m_neckInfo.lookTime = t;
	if(m_neckInfo.lookTargetType == LookTargetType::Entity && m_neckInfo.hEntityLookTarget.get() == &ent)
		return;
	auto pTrComponentEnt = ent.GetTransformComponent();
	m_neckInfo.lookTargetType = LookTargetType::Entity;
	m_neckInfo.lookTarget = pTrComponentEnt ? pTrComponentEnt->GetEyePosition() : Vector3 {};
	m_neckInfo.hEntityLookTarget = ent.GetHandle();

	OnLookTargetChanged();
}
Vector3 BaseAIComponent::GetLookTarget() const
{
	switch(m_neckInfo.lookTargetType) {
	case LookTargetType::Position:
		return m_neckInfo.lookTarget;
	case LookTargetType::Entity:
		{
			if(m_neckInfo.hEntityLookTarget.valid() == false)
				return uvec::PRM_ORIGIN;
			return m_neckInfo.hEntityLookTarget.get()->GetTransformComponent() ? m_neckInfo.hEntityLookTarget.get()->GetCenter() : uvec::PRM_ORIGIN;
		}
	default:
		return uvec::PRM_ORIGIN;
	}
}

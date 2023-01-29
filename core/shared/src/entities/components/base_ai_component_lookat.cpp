/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_ai_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/model/model.h"

using namespace pragma;

void BaseAIComponent::LookAtStep(float tDelta)
{
	// TODO: Implement forward / inverse kinematics and do this properly
	if((m_neckInfo.lookTargetType == BaseAIComponent::LookTargetType::None && m_neckInfo.neckTurned == false))
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
	if(m_neckInfo.lookTargetType != BaseAIComponent::LookTargetType::None) {
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
			valTgt = umath::clamp_angle(valTgt, blendController->min, blendController->max);

		auto angDiff = umath::get_angle_difference(val, valTgt);
		auto &turnSpeed = m_neckInfo.turnSpeed[i];
		if((umath::abs(angDiff) / umath::abs(turnSpeed)) < fadeTime) // We'll have reached the turn target soon, slow down (TODO: This might cause jittering)
			turnSpeed = umath::approach(m_neckInfo.turnSpeed[i], 0.f, turnAcceleration * tDelta);
		else
			turnSpeed = umath::approach(m_neckInfo.turnSpeed[i], maxTurnSpeed * static_cast<float>(umath::sign(valTgt)), turnAcceleration * tDelta);
		val = umath::approach_angle(val, valTgt, tDelta * umath::abs(turnSpeed));
		animComponent->SetBlendController(blendControllers[i], val);
		if(val != 0.f)
			m_neckInfo.neckTurned = true;
	}
}
void BaseAIComponent::ClearLookTarget()
{
	if(m_neckInfo.lookTargetType == BaseAIComponent::LookTargetType::None)
		return;
	m_neckInfo.lookTargetType = BaseAIComponent::LookTargetType::None;
	m_neckInfo.lookTarget = {};
	m_neckInfo.hEntityLookTarget = EntityHandle {};
	m_neckInfo.lookTime = std::numeric_limits<float>::max();
	OnLookTargetChanged();
}
void BaseAIComponent::SetLookTarget(const Vector3 &pos, float t)
{
	m_neckInfo.lookTime = t;
	if(m_neckInfo.lookTargetType == BaseAIComponent::LookTargetType::Position && uvec::cmp(pos, m_neckInfo.lookTarget) == true)
		return;
	m_neckInfo.lookTargetType = BaseAIComponent::LookTargetType::Position;
	m_neckInfo.lookTarget = pos;
	m_neckInfo.hEntityLookTarget = EntityHandle {};

	OnLookTargetChanged();
}
void BaseAIComponent::SetLookTarget(const BaseEntity &ent, float t)
{
	m_neckInfo.lookTime = t;
	if(m_neckInfo.lookTargetType == BaseAIComponent::LookTargetType::Entity && m_neckInfo.hEntityLookTarget.get() == &ent)
		return;
	auto pTrComponentEnt = ent.GetTransformComponent();
	m_neckInfo.lookTargetType = BaseAIComponent::LookTargetType::Entity;
	m_neckInfo.lookTarget = pTrComponentEnt ? pTrComponentEnt->GetEyePosition() : Vector3 {};
	m_neckInfo.hEntityLookTarget = ent.GetHandle();

	OnLookTargetChanged();
}
Vector3 BaseAIComponent::GetLookTarget() const
{
	switch(m_neckInfo.lookTargetType) {
	case BaseAIComponent::LookTargetType::Position:
		return m_neckInfo.lookTarget;
	case BaseAIComponent::LookTargetType::Entity:
		{
			if(m_neckInfo.hEntityLookTarget.valid() == false)
				return uvec::ORIGIN;
			return m_neckInfo.hEntityLookTarget.get()->GetTransformComponent() ? m_neckInfo.hEntityLookTarget.get()->GetCenter() : uvec::ORIGIN;
		}
	default:
		return uvec::ORIGIN;
	}
}

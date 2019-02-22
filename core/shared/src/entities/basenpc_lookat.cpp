#include "stdafx_shared.h"
#if 0
#include "pragma/entities/basenpc.h"

void BaseNPC::LookAtStep(float tDelta)
{
	// TODO: Implement forward / inverse kinematics and do this properly
	if((m_neckInfo.lookTargetType == BaseNPC::LookTargetType::None && m_neckInfo.neckTurned == false))
		return;
	auto *nw = m_entity->GetNetworkState();
	auto *game = nw->GetGameState();
	auto t = game->CurTime();
	if(t > m_neckInfo.lookTime)
		ClearLookTarget();
	Vector3 pos = {};
	auto rot = uquat::identity();
	pos = m_entity->GetEyePosition();
	rot = m_entity->GetOrientation();
	//if(m_entity->GetLocalBonePosition(m_neckInfo.boneId,pos,rot) == false)
	//	return;
	//m_entity->LocalToWorld(&pos,&rot);

	EulerAngles ang {};
	if(m_neckInfo.lookTargetType != BaseNPC::LookTargetType::None)
	{
		auto tgtPos = GetLookTarget();
		auto dir = tgtPos -pos;
		uvec::normalize(&dir);

		auto rotInv = m_entity->GetOrientation();
		uquat::inverse(rotInv);
		ang = EulerAngles(rotInv *uquat::create_look_rotation(dir,m_entity->GetUp()));
		// Deprecated
		// ang = uvec::to_angle(dir,m_entity->GetUp()) -m_entity->GetAngles();
	}
	auto &mdl = m_entity->GetModelComponent().GetModel();
	const auto maxTurnSpeed = 320.f;
	const auto fadeTime = 0.1f;
	auto turnAcceleration = maxTurnSpeed /fadeTime; // Reach full acceleration after x seconds
	m_neckInfo.neckTurned = false;
	std::array<int32_t,2> blendControllers = {GetNeckPitchBlendController(),GetNeckYawBlendController()};
	for(auto i=decltype(blendControllers.size()){0};i<blendControllers.size();++i)
	{
		auto val = m_entity->GetModelComponent().GetBlendController(blendControllers[i]);
		auto *blendController = (mdl != nullptr) ? mdl->GetBlendController(blendControllers[i]) : nullptr;
		auto valTgt = ang[i];

		if(blendController != nullptr)
			valTgt = umath::clamp_angle(valTgt,blendController->min,blendController->max);

		auto angDiff = umath::get_angle_difference(val,valTgt);
		auto &turnSpeed = m_neckInfo.turnSpeed[i];
		if((umath::abs(angDiff) /umath::abs(turnSpeed)) < fadeTime) // We'll have reached the turn target soon, slow down (TODO: This might cause jittering)
			turnSpeed = umath::approach(m_neckInfo.turnSpeed[i],0.f,turnAcceleration *tDelta);
		else
			turnSpeed = umath::approach(m_neckInfo.turnSpeed[i],maxTurnSpeed *static_cast<float>(umath::sign(valTgt)),turnAcceleration *tDelta);
		val = umath::approach_angle(val,valTgt,tDelta *umath::abs(turnSpeed));
		m_entity->GetModelComponent().SetBlendController(blendControllers[i],val);
		if(val != 0.f)
			m_neckInfo.neckTurned = true;
	}
}
void BaseNPC::ClearLookTarget()
{
	if(m_neckInfo.lookTargetType == BaseNPC::LookTargetType::None)
		return;
	m_neckInfo.lookTargetType = BaseNPC::LookTargetType::None;
	m_neckInfo.lookTarget = {};
	m_neckInfo.hEntityLookTarget = {};
	m_neckInfo.lookTime = std::numeric_limits<float>::max();
	OnLookTargetChanged();
}
void BaseNPC::SetLookTarget(const Vector3 &pos,float t)
{
	m_neckInfo.lookTime = t;
	if(m_neckInfo.lookTargetType == BaseNPC::LookTargetType::Position && uvec::cmp(pos,m_neckInfo.lookTarget) == true)
		return;
	m_neckInfo.lookTargetType = BaseNPC::LookTargetType::Position;
	m_neckInfo.lookTarget = pos;
	m_neckInfo.hEntityLookTarget = {};

	OnLookTargetChanged();
}
void BaseNPC::SetLookTarget(const BaseEntity &ent,float t)
{
	m_neckInfo.lookTime = t;
	if(m_neckInfo.lookTargetType == BaseNPC::LookTargetType::Entity && m_neckInfo.hEntityLookTarget.get() == &ent)
		return;
	m_neckInfo.lookTargetType = BaseNPC::LookTargetType::Entity;
	m_neckInfo.lookTarget = ent.GetEyePosition();
	m_neckInfo.hEntityLookTarget = ent.GetHandle();

	OnLookTargetChanged();
}
Vector3 BaseNPC::GetLookTarget() const
{
	switch(m_neckInfo.lookTargetType)
	{
		case BaseNPC::LookTargetType::Position:
			return m_neckInfo.lookTarget;
		case BaseNPC::LookTargetType::Entity:
			return (m_neckInfo.hEntityLookTarget.IsValid() == true) ? m_neckInfo.hEntityLookTarget.get()->GetCenter() : uvec::ORIGIN;
		default:
			return uvec::ORIGIN;
	}
}
#endif

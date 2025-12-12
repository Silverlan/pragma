// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.point.constraints.base_cone_twist;

using namespace pragma;

void BasePointConstraintConeTwistComponent::Initialize()
{
	BasePointConstraintComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "swingspan1", false))
			m_kvSwingSpan1 = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "swingspan2", false))
			m_kvSwingSpan2 = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "twistspan", false))
			m_kvTwistSpan = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "softness", false))
			m_kvSoftness = pragma::util::to_float(kvData.value);
		// else if(pragma::string::compare<std::string>(kvData.key,"biasfactor",false))
		// 	m_kvBiasFactor = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "relaxationfactor", false))
			m_kvRelaxationFactor = pragma::util::to_float(kvData.value);
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});
}

void BasePointConstraintConeTwistComponent::InitializeConstraint(pragma::ecs::BaseEntity *src, pragma::ecs::BaseEntity *tgt)
{
	auto &entThis = GetEntity();
	auto *state = entThis.GetNetworkState();
	auto *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();

	auto pPhysComponentSrc = src->GetPhysicsComponent();
	auto *physSrc = pPhysComponentSrc ? dynamic_cast<pragma::physics::RigidPhysObj *>(pPhysComponentSrc->GetPhysicsObject()) : nullptr;
	auto pPhysComponentTgt = tgt->GetPhysicsComponent();
	auto *physTgt = pPhysComponentTgt ? dynamic_cast<pragma::physics::RigidPhysObj *>(pPhysComponentTgt->GetPhysicsObject()) : nullptr;
	if(physSrc == nullptr || physTgt == nullptr || !physSrc->IsRigid() || !physTgt->IsRigid())
		return;
	auto *rigidSrc = static_cast<pragma::physics::RigidPhysObj *>(physSrc)->GetRigidBody();
	auto *rigidTgt = static_cast<pragma::physics::RigidPhysObj *>(physTgt)->GetRigidBody();
	if(rigidSrc == nullptr || rigidTgt == nullptr)
		return;

	auto pTrComponent = entThis.GetTransformComponent();
	auto pTrComponentTgt = tgt->GetTransformComponent();
	auto originConstraint = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	auto originTgt = pTrComponentTgt ? pTrComponentTgt->GetPosition() : Vector3 {};

	auto rotConstraint = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
	auto rotTgt = pTrComponentTgt ? pTrComponentTgt->GetRotation() : uquat::identity();

	auto pTrComponentSrc = src->GetTransformComponent();
	originTgt = originConstraint - originTgt;
	originConstraint -= pTrComponentSrc ? pTrComponentSrc->GetPosition() : Vector3 {};

	auto swingSpan1 = CFloat(pragma::math::deg_to_rad(m_kvSwingSpan1));
	auto swingSpan2 = CFloat(pragma::math::deg_to_rad(m_kvSwingSpan2));
	auto twistSpan = CFloat(pragma::math::deg_to_rad(m_kvTwistSpan));

	auto coneTwist = physEnv->CreateConeTwistConstraint(*rigidSrc, originConstraint, rotConstraint, *rigidTgt, originTgt, rotTgt);
	if(coneTwist.IsValid()) {
		coneTwist->SetEntity(GetEntity());
		coneTwist->SetLimit(swingSpan1, swingSpan2, twistSpan);
		coneTwist->SetSoftness(m_kvSoftness);
		coneTwist->SetDamping(m_kvRelaxationFactor);
		m_constraints.push_back(pragma::util::shared_handle_cast<physics::IConeTwistConstraint, physics::IConstraint>(coneTwist));
	}
}

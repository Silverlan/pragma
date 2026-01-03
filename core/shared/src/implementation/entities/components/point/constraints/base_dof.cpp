// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.point.constraints.base_dof;

using namespace pragma;

void BasePointConstraintDoFComponent::Initialize()
{
	BasePointConstraintComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "limit_lin_lower", false))
			m_kvLimLinLower = uvec::create(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "limit_lin_upper", false))
			m_kvLimLinUpper = uvec::create(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "limit_ang_lower", false))
			m_kvLimAngLower = uvec::create(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "limit_ang_upper", false))
			m_kvLimAngUpper = uvec::create(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BasePointConstraintDoFComponent::InitializeConstraint(ecs::BaseEntity *src, ecs::BaseEntity *tgt)
{
	auto pPhysComponentTgt = tgt->GetPhysicsComponent();
	auto *physTgt = pPhysComponentTgt ? dynamic_cast<physics::RigidPhysObj *>(pPhysComponentTgt->GetPhysicsObject()) : nullptr;
	if(physTgt == nullptr)
		return;
	auto pPhysComponentSrc = src->GetPhysicsComponent();
	auto *physSrc = pPhysComponentSrc ? dynamic_cast<physics::RigidPhysObj *>(pPhysComponentSrc->GetPhysicsObject()) : nullptr;
	if(physSrc == nullptr)
		return;
	auto *bodySrc = physSrc->GetRigidBody();
	if(bodySrc == nullptr)
		return;
	auto &entThis = GetEntity();
	auto *state = entThis.GetNetworkState();
	auto *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	//auto &posThis = entThis.GetPosition();
	auto pTrComponent = entThis.GetTransformComponent();
	auto dir = pTrComponent != nullptr ? pTrComponent->GetForward() : uvec::PRM_FORWARD;

	auto &bodies = physTgt->GetRigidBodies();
	for(auto it = bodies.begin(); it != bodies.end(); ++it) {
		auto &bodyTgt = *it;
		if(bodyTgt.IsValid()) {
			auto posTgt = bodyTgt->GetPos();
			auto dof = physEnv->CreateDoFConstraint(*bodySrc, Vector3(0.f, 0.f, 0.f), uquat::identity(), *bodyTgt, Vector3(0.f, 50.f, 0.f), uquat::identity());
			if(dof != nullptr) {
				dof->SetEntity(GetEntity());
				//dof->SetLinearLimit(Vector3(1.f,1.f,1.f),Vector3(-1.f,-1.f,-1.f));
				dof->SetAngularLimit(EulerAngles(180, 180, 180), EulerAngles(-180, -180, -180));
				//dof->SetLinearLimit(m_kvLimLinLower,m_kvLimLinUpper);
				//dof->SetAngularLimit(m_kvLimAngLower,m_kvLimAngUpper);
				m_constraints.push_back(pragma::util::shared_handle_cast<physics::IDoFConstraint, physics::IConstraint>(dof));
			}
		}
	}
}

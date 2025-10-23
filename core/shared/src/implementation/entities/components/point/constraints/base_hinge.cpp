// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <string>

module pragma.shared;

import :entities.components.point.constraints.base_hinge;

using namespace pragma;

void BasePointConstraintHingeComponent::Initialize()
{
	BasePointConstraintComponent::Initialize();

	BindEvent(pragma::ecs::BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "limit_low", false))
			m_kvLimitLow = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "limit_high", false))
			m_kvLimitHigh = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "softness", false))
			m_kvLimitSoftness = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "biasfactor", false))
			m_kvLimitBiasFactor = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "relaxationfactor", false))
			m_kvLimitRelaxationFactor = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BasePointConstraintHingeComponent::InitializeConstraint(pragma::ecs::BaseEntity *src, pragma::ecs::BaseEntity *tgt)
{
	auto pPhysComponentTgt = tgt->GetPhysicsComponent();
	auto *physTgt = pPhysComponentTgt ? dynamic_cast<RigidPhysObj *>(pPhysComponentTgt->GetPhysicsObject()) : nullptr;
	if(physTgt == nullptr)
		return;
	auto pPhysComponentSrc = src->GetPhysicsComponent();
	auto *physSrc = pPhysComponentSrc ? dynamic_cast<RigidPhysObj *>(pPhysComponentSrc->GetPhysicsObject()) : nullptr;
	if(physSrc == nullptr)
		return;
	auto *bodySrc = physSrc->GetRigidBody();
	if(bodySrc == nullptr)
		return;
	auto &entThis = GetEntity();
	auto *state = entThis.GetNetworkState();
	auto *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	auto pTrComponent = entThis.GetTransformComponent();
	auto posThis = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	auto axis = m_posTarget - posThis;
	uvec::normalize(&axis);

	auto &bodies = physTgt->GetRigidBodies();
	for(auto it = bodies.begin(); it != bodies.end(); ++it) {
		auto &bodyTgt = *it;
		if(bodyTgt.IsValid()) {
			auto posTgt = bodyTgt->GetPos();
			auto hinge = physEnv->CreateHingeConstraint(*bodyTgt, posThis - posTgt, *bodySrc, posThis, axis);
			if(hinge != nullptr) {
				hinge->SetEntity(GetEntity());
				m_constraints.push_back(util::shared_handle_cast<pragma::physics::IHingeConstraint, pragma::physics::IConstraint>(hinge));
			}
		}
	}
}

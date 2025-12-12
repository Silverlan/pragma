// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.props.base_physics;

using namespace pragma;

void BasePropPhysicsComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &ent = GetEntity();
		auto mdlComponent = ent.GetModelComponent();
		if(!mdlComponent || mdlComponent->HasModel() == false || !ent.IsSpawned())
			return pragma::util::EventReply::Unhandled;
		auto *pPropComponent = static_cast<pragma::BasePropComponent *>(ent.FindComponent("prop").get());
		if(pPropComponent != nullptr)
			pPropComponent->UpdatePhysicsType(&ent);
		return pragma::util::EventReply::Unhandled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("prop");
}

void BasePropPhysicsComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto *pPropComponent = static_cast<pragma::BasePropComponent *>(GetEntity().FindComponent("prop").get());
	auto physType = pragma::physics::PhysicsType::None;
	if(pPropComponent == nullptr)
		return;
	physType = pPropComponent->UpdatePhysicsType(&GetEntity());
	pPropComponent->Setup(physType, pragma::physics::MoveType::Physics);
}

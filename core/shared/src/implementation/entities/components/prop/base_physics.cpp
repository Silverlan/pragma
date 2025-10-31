// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;



module pragma.shared;

import :entities.components.props.base_physics;

using namespace pragma;

void BasePropPhysicsComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &ent = GetEntity();
		auto mdlComponent = ent.GetModelComponent();
		if(!mdlComponent || mdlComponent->HasModel() == false || !ent.IsSpawned())
			return util::EventReply::Unhandled;
		auto *pPropComponent = static_cast<pragma::BasePropComponent *>(ent.FindComponent("prop").get());
		if(pPropComponent != nullptr)
			pPropComponent->UpdatePhysicsType(&ent);
		return util::EventReply::Unhandled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("prop");
}

void BasePropPhysicsComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto *pPropComponent = static_cast<pragma::BasePropComponent *>(GetEntity().FindComponent("prop").get());
	auto physType = pragma::physics::PHYSICSTYPE::NONE;
	if(pPropComponent == nullptr)
		return;
	physType = pPropComponent->UpdatePhysicsType(&GetEntity());
	pPropComponent->Setup(physType, pragma::physics::MOVETYPE::PHYSICS);
}

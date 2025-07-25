// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/baseworld.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include <pragma/physics/collisiontypes.h>

using namespace pragma;

void BaseWorldComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);
	ent.AddComponent("model");
	ent.AddComponent("physics");

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { InitializePhysics(); });
}

void BaseWorldComponent::InitializePhysics()
{
	auto &ent = GetEntity();
	if(ent.IsSpawned() == false)
		return;
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(!pPhysComponent)
		return;
	auto mdl = ent.GetModel();
	if(mdl)
		pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
	else
		pPhysComponent->DestroyPhysicsObject();
}

void BaseWorldComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	GetEntity().GetNetworkState()->GetGameState()->SetWorld(this);
	InitializePhysics();
}

Con::c_cout &BaseWorldComponent::print(Con::c_cout &os)
{
	os << "World";
	return os;
}

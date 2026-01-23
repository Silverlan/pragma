// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_world;

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

	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { InitializePhysics(); });
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
		pPhysComponent->InitializePhysics(physics::PhysicsType::Static);
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

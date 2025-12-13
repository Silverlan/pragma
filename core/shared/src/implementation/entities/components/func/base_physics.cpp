// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.func.base_physics;

using namespace pragma;

void BaseFuncPhysicsComponent::Initialize()
{
	BaseFuncSurfaceMaterialComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "mass", false))
			m_kvMass = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "surface_material", false))
			m_kvSurfaceMaterial = kvData.value;
		else if(pragma::string::compare<std::string>(kvData.key, "clientside_physics", false))
			m_bClientsidePhysics = util::to_boolean(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(basePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto pPhysComponent = ent.GetPhysicsComponent();
		auto *phys = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
		if(phys != nullptr)
			phys->SetMass(m_kvMass);
	});

	auto &ent = GetEntity();
	ent.AddComponent("physics");
	ent.AddComponent("name");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);
	ent.AddComponent("model");
}

void BaseFuncPhysicsComponent::OnEntitySpawn()
{
	BaseFuncSurfaceMaterialComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr) {
		pPhysComponent->DestroyPhysicsObject();
		pPhysComponent->SetMoveType(physics::MoveType::Physics);
	}
	auto *phys = InitializePhysics();
	if(phys != nullptr) {
		UpdateSurfaceMaterial(ent.GetNetworkState()->GetGameState());
		phys->WakeUp();
	}
}

physics::PhysObj *BaseFuncPhysicsComponent::InitializePhysics()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return nullptr;
	return pPhysComponent->InitializePhysics((m_kvMass > 0) ? physics::PhysicsType::Dynamic : physics::PhysicsType::Static);
}

#include "stdafx_shared.h"
#include "pragma/entities/baseskybox.h"
#include "pragma/entities/components/base_physics_component.hpp"

using namespace pragma;

void BaseSkyboxComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("render");
	ent.AddComponent("physics");
	ent.AddComponent("model");
}

void BaseSkyboxComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent.expired())
		return;
	pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
}

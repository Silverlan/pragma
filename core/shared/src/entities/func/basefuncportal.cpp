#include "stdafx_shared.h"
#include "pragma/entities/func/basefuncportal.h"
#include "pragma/entities/components/base_physics_component.hpp"

using namespace pragma;

void BaseFuncPortalComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("physics");
	ent.AddComponent("name");
}

void BaseFuncPortalComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent.expired())
		return;
	pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
}

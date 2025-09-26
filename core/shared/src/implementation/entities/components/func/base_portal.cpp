// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/entities/func/basefuncportal.h"
#include "pragma/entities/components/base_physics_component.hpp"

module pragma.shared;

import :entities.components.func.base_portal;

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
	if(pPhysComponent == nullptr)
		return;
	pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
}

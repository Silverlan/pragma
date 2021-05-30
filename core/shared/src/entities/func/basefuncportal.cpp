/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
	if(pPhysComponent == nullptr)
		return;
	pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
}

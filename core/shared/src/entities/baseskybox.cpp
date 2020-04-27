/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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

	m_netEvSetSkyAngles = SetupNetEvent("set_sky_angles");
}

void BaseSkyboxComponent::SetSkyAngles(const EulerAngles &ang) {m_skyAngles = ang;}
const EulerAngles &BaseSkyboxComponent::GetSkyAngles() const {return m_skyAngles;}

void BaseSkyboxComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent.expired())
		return;
	pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
}

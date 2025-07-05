// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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

void BaseSkyboxComponent::SetSkyAngles(const EulerAngles &ang) { m_skyAngles = ang; }
const EulerAngles &BaseSkyboxComponent::GetSkyAngles() const { return m_skyAngles; }

void BaseSkyboxComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(!pPhysComponent)
		return;
	pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
}

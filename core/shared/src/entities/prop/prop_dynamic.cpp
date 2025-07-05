// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/prop/prop_dynamic.hpp"

using namespace pragma;

void BasePropDynamicComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("prop");
	ent.RemoveComponent("physics");
}

void BasePropDynamicComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/prop/prop_dynamic.hpp"

using namespace pragma;

void BasePropDynamicComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("prop");
}

void BasePropDynamicComponent::OnEntitySpawn() {BaseEntityComponent::OnEntitySpawn();}

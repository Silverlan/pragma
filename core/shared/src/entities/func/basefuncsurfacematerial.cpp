/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/entities/func/basefuncsurfacematerial.hpp"
#include "pragma/entities/components/base_physics_component.hpp"

using namespace pragma;

void BaseFuncSurfaceMaterialComponent::UpdateSurfaceMaterial(Game *game)
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr)
		return;
	auto *mat = game->GetSurfaceMaterial(m_kvSurfaceMaterial);
	if(mat == nullptr)
		return;
	auto idx = mat->GetIndex();
	auto &objs = phys->GetCollisionObjects();
	for(auto it = objs.begin(); it != objs.end(); ++it) {
		auto &hObj = *it;
		if(hObj.IsValid())
			hObj->SetSurfaceMaterial(CUInt32(idx));
	}
}

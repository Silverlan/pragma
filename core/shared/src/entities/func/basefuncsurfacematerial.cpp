// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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

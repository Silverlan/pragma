// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.func.base_surface_material;

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

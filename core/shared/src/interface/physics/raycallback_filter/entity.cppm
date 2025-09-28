// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:physics.raycallback.filter_entity;

export import :physics.raycallback.filter;
export import :entities.base_entity_handle;

export class DLLNETWORK BasePhysRayCallbackFilterEntity : public BasePhysRayCallbackFilter {
  protected:
	std::vector<EntityHandle> m_filter;
  public:
	BasePhysRayCallbackFilterEntity(const std::vector<EntityHandle> &filter, RayCastFlags flags, CollisionMask group, CollisionMask mask);
	BasePhysRayCallbackFilterEntity(const EntityHandle &filter, RayCastFlags flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
};

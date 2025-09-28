// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:physics.raycallback.filter_collision_object;

export import :physics.raycallback.filter;

export class DLLNETWORK BasePhysRayCallbackFilterCollisionObject : public BasePhysRayCallbackFilter {
  protected:
	std::vector<pragma::physics::ICollisionObject *> m_filter;
  public:
	BasePhysRayCallbackFilterCollisionObject(const std::vector<pragma::physics::ICollisionObject *> &filter, RayCastFlags flags, CollisionMask group, CollisionMask mask);
	BasePhysRayCallbackFilterCollisionObject(pragma::physics::ICollisionObject *filter, RayCastFlags flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
};

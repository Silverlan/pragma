// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:physics.raycallback.filter_phys_obj;

export import :physics.raycallback.filter;

export class DLLNETWORK BasePhysRayCallbackFilterPhysObj : public BasePhysRayCallbackFilter {
  protected:
	std::vector<PhysObjHandle> m_filter;
  public:
	BasePhysRayCallbackFilterPhysObj(const std::vector<PhysObjHandle> &filter, RayCastFlags flags, CollisionMask group, CollisionMask mask);
	BasePhysRayCallbackFilterPhysObj(const PhysObjHandle &filter, RayCastFlags flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
};

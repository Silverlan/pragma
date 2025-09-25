// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/physics/physobj.h"

export module pragma.shared:physics.raycallback.filter_phys_obj;

export class DLLNETWORK BasePhysRayCallbackFilterPhysObj : public BasePhysRayCallbackFilter {
  protected:
	std::vector<PhysObjHandle> m_filter;
  public:
	BasePhysRayCallbackFilterPhysObj(const std::vector<PhysObjHandle> &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	BasePhysRayCallbackFilterPhysObj(const PhysObjHandle &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
};

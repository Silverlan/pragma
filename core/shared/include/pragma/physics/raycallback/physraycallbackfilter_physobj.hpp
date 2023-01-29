/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYS_RAYCALLBACKFILTER_PHYSOBJ_HPP__
#define __PHYS_RAYCALLBACKFILTER_PHYSOBJ_HPP__

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/physics/physobj.h"

class DLLNETWORK BasePhysRayCallbackFilterPhysObj : public BasePhysRayCallbackFilter {
  protected:
	std::vector<PhysObjHandle> m_filter;
  public:
	BasePhysRayCallbackFilterPhysObj(const std::vector<PhysObjHandle> &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	BasePhysRayCallbackFilterPhysObj(const PhysObjHandle &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
};

#endif

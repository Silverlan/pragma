/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYS_RAYCALLBACKFILTER_COLLISIONOBJECT_HPP__
#define __PHYS_RAYCALLBACKFILTER_COLLISIONOBJECT_HPP__

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"

class DLLNETWORK BasePhysRayCallbackFilterCollisionObject : public BasePhysRayCallbackFilter {
  protected:
	std::vector<pragma::physics::ICollisionObject *> m_filter;
  public:
	BasePhysRayCallbackFilterCollisionObject(const std::vector<pragma::physics::ICollisionObject *> &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	BasePhysRayCallbackFilterCollisionObject(pragma::physics::ICollisionObject *filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
};

#endif

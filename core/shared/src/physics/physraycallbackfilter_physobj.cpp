/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter_physobj.hpp"

BasePhysRayCallbackFilterPhysObj::BasePhysRayCallbackFilterPhysObj(const std::vector<PhysObjHandle> &filter, FTRACE flags, CollisionMask group, CollisionMask mask) : BasePhysRayCallbackFilter(flags, group, mask), m_filter(filter) {}
BasePhysRayCallbackFilterPhysObj::BasePhysRayCallbackFilterPhysObj(const PhysObjHandle &filter, FTRACE flags, CollisionMask group, CollisionMask mask) : BasePhysRayCallbackFilter(flags, group, mask) { m_filter.push_back(filter); }
bool BasePhysRayCallbackFilterPhysObj::ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *col)
{
	if(BasePhysRayCallbackFilter::ShouldPass(ent, phys, col) == false)
		return false;
	for(auto it = m_filter.begin(); it != m_filter.end(); it++) {
		if(it->get() == phys)
			return TranslateFilterValue(true);
	}
	return TranslateFilterValue(false);
}

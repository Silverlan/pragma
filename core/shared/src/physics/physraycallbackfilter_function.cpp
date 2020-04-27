/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter_function.hpp"

BasePhysRayCallbackFilterFunction::BasePhysRayCallbackFilterFunction(const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*)> &filter,FTRACE flags,CollisionMask group,CollisionMask mask)
	: BasePhysRayCallbackFilter(flags,group,mask),m_filter(filter)
{}
bool BasePhysRayCallbackFilterFunction::ShouldPass(BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *col)
{
	if(BasePhysRayCallbackFilter::ShouldPass(ent,phys,col) == false)
		return false;
	return TranslateFilterValue(m_filter(ent,phys,col));
}
void BasePhysRayCallbackFilterFunction::SetFilter(const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*)> &f) {m_filter = f;}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter_collisionobject.hpp"

BasePhysRayCallbackFilterCollisionObject::BasePhysRayCallbackFilterCollisionObject(const std::vector<pragma::physics::ICollisionObject*> &filter,FTRACE flags,CollisionMask group,CollisionMask mask)
	: BasePhysRayCallbackFilter(flags,group,mask),m_filter(filter)
{}
BasePhysRayCallbackFilterCollisionObject::BasePhysRayCallbackFilterCollisionObject(pragma::physics::ICollisionObject *filter,FTRACE flags,CollisionMask group,CollisionMask mask)
	: BasePhysRayCallbackFilter(flags,group,mask)
{
	m_filter.push_back(filter);
}
bool BasePhysRayCallbackFilterCollisionObject::ShouldPass(BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *col)
{
	if(BasePhysRayCallbackFilter::ShouldPass(ent,phys,col) == false)
		return false;
	for(auto it=m_filter.begin();it!=m_filter.end();it++)
	{
		if(*it == col)
			return TranslateFilterValue(true);
	}
	return TranslateFilterValue(false);
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __PHYS_RAYCALLBACKFILTER_ENTITY_HPP__
#define __PHYS_RAYCALLBACKFILTER_ENTITY_HPP__

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/entities/baseentity_handle.h"

class DLLNETWORK BasePhysRayCallbackFilterEntity
	: public BasePhysRayCallbackFilter
{
protected:
	std::vector<EntityHandle> m_filter;
public:
	BasePhysRayCallbackFilterEntity(const std::vector<EntityHandle> &filter,FTRACE flags,CollisionMask group,CollisionMask mask);
	BasePhysRayCallbackFilterEntity(const EntityHandle &filter,FTRACE flags,CollisionMask group,CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *obj) override;
};

#endif

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYS_RAYCALLBACKFILTER_HPP__
#define __PHYS_RAYCALLBACKFILTER_HPP__

#include "pragma/lua/luafunction.h"
#include "pragma/physics/collisionmasks.h"

DLLNETWORK CollisionMask get_collision_group(CollisionMask group,CollisionMask mask);

enum class FTRACE;
class BaseEntity;
class PhysObj;
namespace pragma::physics {class ICollisionObject;};
class DLLNETWORK BasePhysRayCallbackFilter
{
protected:
	FTRACE m_flags;
	CollisionMask m_filterMask;
	CollisionMask m_filterGroup;
	mutable void *m_userData = nullptr;
	BasePhysRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask);
	bool TranslateFilterValue(bool b) const;
public:
	virtual bool ShouldPass(BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *obj);
	void SetUserData(void *userData) const;
	void *GetUserData() const;
};

#endif

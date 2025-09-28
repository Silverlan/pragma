// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.shared;

import :physics.raycallback.filter_collision_object;

BasePhysRayCallbackFilterCollisionObject::BasePhysRayCallbackFilterCollisionObject(const std::vector<pragma::physics::ICollisionObject *> &filter, FTRACE flags, CollisionMask group, CollisionMask mask) : BasePhysRayCallbackFilter(flags, group, mask), m_filter(filter) {}
BasePhysRayCallbackFilterCollisionObject::BasePhysRayCallbackFilterCollisionObject(pragma::physics::ICollisionObject *filter, FTRACE flags, CollisionMask group, CollisionMask mask) : BasePhysRayCallbackFilter(flags, group, mask) { m_filter.push_back(filter); }
bool BasePhysRayCallbackFilterCollisionObject::ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *col)
{
	if(BasePhysRayCallbackFilter::ShouldPass(ent, phys, col) == false)
		return false;
	for(auto it = m_filter.begin(); it != m_filter.end(); it++) {
		if(*it == col)
			return TranslateFilterValue(true);
	}
	return TranslateFilterValue(false);
}

#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter_entity.hpp"

BasePhysRayCallbackFilterEntity::BasePhysRayCallbackFilterEntity(const std::vector<EntityHandle> &filter,FTRACE flags,CollisionMask group,CollisionMask mask)
	: BasePhysRayCallbackFilter(flags,group,mask),m_filter(filter)
{}
BasePhysRayCallbackFilterEntity::BasePhysRayCallbackFilterEntity(const EntityHandle &filter,FTRACE flags,CollisionMask group,CollisionMask mask)
	: BasePhysRayCallbackFilter(flags,group,mask)
{
	m_filter.push_back(filter);
}
bool BasePhysRayCallbackFilterEntity::ShouldPass(BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *col)
{
	if(BasePhysRayCallbackFilter::ShouldPass(ent,phys,col) == false)
		return false;
	for(auto it=m_filter.begin();it!=m_filter.end();it++)
	{
		if(it->get() == ent)
			return TranslateFilterValue(true);
	}
	return TranslateFilterValue(false);
}

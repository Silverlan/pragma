#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter_collisionobject.hpp"

BasePhysRayCallbackFilterCollisionObject::BasePhysRayCallbackFilterCollisionObject(const std::vector<PhysCollisionObject*> &filter,FTRACE flags,CollisionMask group,CollisionMask mask)
	: BasePhysRayCallbackFilter(flags,group,mask),m_filter(filter)
{}
BasePhysRayCallbackFilterCollisionObject::BasePhysRayCallbackFilterCollisionObject(PhysCollisionObject *filter,FTRACE flags,CollisionMask group,CollisionMask mask)
	: BasePhysRayCallbackFilter(flags,group,mask)
{
	m_filter.push_back(filter);
}
bool BasePhysRayCallbackFilterCollisionObject::ShouldPass(BaseEntity *ent,PhysObj *phys,PhysCollisionObject *col)
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

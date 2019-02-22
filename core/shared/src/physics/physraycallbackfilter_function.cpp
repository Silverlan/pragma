#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter_function.hpp"

BasePhysRayCallbackFilterFunction::BasePhysRayCallbackFilterFunction(const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*)> &filter,FTRACE flags,CollisionMask group,CollisionMask mask)
	: BasePhysRayCallbackFilter(flags,group,mask),m_filter(filter)
{}
bool BasePhysRayCallbackFilterFunction::ShouldPass(BaseEntity *ent,PhysObj *phys,PhysCollisionObject *col)
{
	if(BasePhysRayCallbackFilter::ShouldPass(ent,phys,col) == false)
		return false;
	return TranslateFilterValue(m_filter(ent,phys,col));
}
void BasePhysRayCallbackFilterFunction::SetFilter(const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*)> &f) {m_filter = f;}

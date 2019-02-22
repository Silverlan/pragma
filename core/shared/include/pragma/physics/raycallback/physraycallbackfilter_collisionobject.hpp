#ifndef __PHYS_RAYCALLBACKFILTER_COLLISIONOBJECT_HPP__
#define __PHYS_RAYCALLBACKFILTER_COLLISIONOBJECT_HPP__

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/physics/physcollisionobject.h"

class DLLNETWORK BasePhysRayCallbackFilterCollisionObject
	: public BasePhysRayCallbackFilter
{
protected:
	std::vector<PhysCollisionObject*> m_filter;
public:
	BasePhysRayCallbackFilterCollisionObject(const std::vector<PhysCollisionObject*> &filter,FTRACE flags,CollisionMask group,CollisionMask mask);
	BasePhysRayCallbackFilterCollisionObject(PhysCollisionObject *filter,FTRACE flags,CollisionMask group,CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent,PhysObj *phys,PhysCollisionObject *obj) override;
};

#endif

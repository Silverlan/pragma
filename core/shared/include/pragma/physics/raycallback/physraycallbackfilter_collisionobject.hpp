#ifndef __PHYS_RAYCALLBACKFILTER_COLLISIONOBJECT_HPP__
#define __PHYS_RAYCALLBACKFILTER_COLLISIONOBJECT_HPP__

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"

class DLLNETWORK BasePhysRayCallbackFilterCollisionObject
	: public BasePhysRayCallbackFilter
{
protected:
	std::vector<pragma::physics::ICollisionObject*> m_filter;
public:
	BasePhysRayCallbackFilterCollisionObject(const std::vector<pragma::physics::ICollisionObject*> &filter,FTRACE flags,CollisionMask group,CollisionMask mask);
	BasePhysRayCallbackFilterCollisionObject(pragma::physics::ICollisionObject *filter,FTRACE flags,CollisionMask group,CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *obj) override;
};

#endif

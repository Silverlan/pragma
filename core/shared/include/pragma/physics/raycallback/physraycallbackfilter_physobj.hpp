#ifndef __PHYS_RAYCALLBACKFILTER_PHYSOBJ_HPP__
#define __PHYS_RAYCALLBACKFILTER_PHYSOBJ_HPP__

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/physics/physobj.h"

class DLLNETWORK BasePhysRayCallbackFilterPhysObj
	: public BasePhysRayCallbackFilter
{
protected:
	std::vector<PhysObjHandle> m_filter;
public:
	BasePhysRayCallbackFilterPhysObj(const std::vector<PhysObjHandle> &filter,FTRACE flags,CollisionMask group,CollisionMask mask);
	BasePhysRayCallbackFilterPhysObj(const PhysObjHandle &filter,FTRACE flags,CollisionMask group,CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent,PhysObj *phys,PhysCollisionObject *obj) override;
};

#endif

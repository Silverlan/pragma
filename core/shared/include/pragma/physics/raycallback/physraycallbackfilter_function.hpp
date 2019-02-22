#ifndef __PHYS_RAYCALLBACKFILTER_FUNCTION_HPP__
#define __PHYS_RAYCALLBACKFILTER_FUNCTION_HPP__

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include <functional>

class DLLNETWORK BasePhysRayCallbackFilterFunction
	: public BasePhysRayCallbackFilter
{
private:
	BasePhysRayCallbackFilterFunction &operator=(const BasePhysRayCallbackFilterFunction&) {return *this;};
protected:
	std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*)> m_filter;
public:
	BasePhysRayCallbackFilterFunction(const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*)> &filter,FTRACE flags,CollisionMask group,CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent,PhysObj *phys,PhysCollisionObject *obj) override;
	void SetFilter(const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*)> &f);
};

#endif

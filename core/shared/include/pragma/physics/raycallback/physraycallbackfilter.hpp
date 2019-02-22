#ifndef __PHYS_RAYCALLBACKFILTER_HPP__
#define __PHYS_RAYCALLBACKFILTER_HPP__

#include <pragma/physics/physapi.h>
#include "pragma/lua/luafunction.h"
#include "pragma/physics/collisionmasks.h"

DLLNETWORK CollisionMask get_collision_group(CollisionMask group,CollisionMask mask);

enum class FTRACE;
class BaseEntity;
class PhysObj;
class PhysCollisionObject;
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
	virtual bool ShouldPass(BaseEntity *ent,PhysObj *phys,PhysCollisionObject *obj);
	void SetUserData(void *userData) const;
	void *GetUserData() const;
};

#endif

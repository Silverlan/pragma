#ifndef __PHYSRAYCALLBACK_CONTACT_HPP__
#define __PHYSRAYCALLBACK_CONTACT_HPP__

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/physics/physobj.h"
#include <functional>

class DLLNETWORK PhysContactResultCallback
	: public btCollisionWorld::ContactResultCallback
{
private:
	std::shared_ptr<BasePhysRayCallbackFilter> m_filter;
public:
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,nullptr_t);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*)> &filter);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*,const btCollisionWorld::LocalRayResult&)> &filter);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*,const btCollisionWorld::LocalConvexResult&)> &filter);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<EntityHandle> &filter);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const EntityHandle &filter);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<PhysObjHandle> &filter);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const PhysObjHandle &filter);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<PhysCollisionObject*> &filter);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,PhysCollisionObject *filter);
	PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const LuaFunction &filter);
	virtual btScalar addSingleResult(btManifoldPoint &cp,const btCollisionObjectWrapper *colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper *colObj1Wrap,int partId1,int index1) override;
	Vector3 m_positionWorldOnA;
	Vector3 m_positionWorldOnB;
	Vector3 m_localPointA;
	Vector3 m_localPointB;
	const btCollisionObject *m_colObj;
	bool hasHit;
};

#endif

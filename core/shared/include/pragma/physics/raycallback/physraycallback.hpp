#ifndef __PHYS_RAYCALLBACK_HPP__
#define __PHYS_RAYCALLBACK_HPP__

// #define ENABLE_DEPRECATED_PHYSICS
#ifdef ENABLE_DEPRECATED_PHYSICS
#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/physics/raycallback/physraycallbackfilter_collisionobject.hpp"
#include "pragma/physics/raycallback/physraycallbackfilter_entity.hpp"
#include "pragma/physics/raycallback/physraycallbackfilter_function.hpp"
#include "pragma/physics/raycallback/physraycallbackfilter_luafunction.hpp"
#include "pragma/physics/raycallback/physraycallbackfilter_physobj.hpp"


template<class T>
	class PhysBaseRayResultCallback
		: public T
{
protected:
	std::shared_ptr<BasePhysRayCallbackFilter> m_filter;
public:
	bool ShouldPass(BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *obj);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,nullptr_t);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*)> &filter);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*,const btCollisionWorld::LocalRayResult&)> &filter);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*,const btCollisionWorld::LocalConvexResult&)> &filter);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<EntityHandle> &filter);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const EntityHandle &filter);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<PhysObjHandle> &filter);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const PhysObjHandle &filter);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<pragma::physics::ICollisionObject*> &filter);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,pragma::physics::ICollisionObject *filter);
	PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const LuaFunction &filter);
};

template<class T>
	bool PhysBaseRayResultCallback<T>::ShouldPass(BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *obj)
{
	if(m_filter == nullptr)
		return true;
	return m_filter->ShouldPass(ent,phys,obj);
}

template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld)
		: T(rayFromWorld,rayToWorld),m_filter(nullptr)
{}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE,CollisionMask,CollisionMask,nullptr_t)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*)> &filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterFunction>(filter,flags,group,mask);
}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*,const btCollisionWorld::LocalRayResult&)> &filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterFunction>(nullptr,flags,group,mask);
	auto *ptrFilter = static_cast<BasePhysRayCallbackFilterFunction*>(m_filter.get());
	ptrFilter->SetFilter([filter,ptrFilter](BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *col) {
		auto *rayResult = static_cast<btCollisionWorld::LocalRayResult*>(ptrFilter->GetUserData());
		return filter(ent,phys,col,*rayResult);
	});
}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*,const btCollisionWorld::LocalConvexResult&)> &filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterFunction>(nullptr,flags,group,mask);
	auto *ptrFilter = static_cast<BasePhysRayCallbackFilterFunction*>(m_filter.get());
	ptrFilter->SetFilter([filter,ptrFilter](BaseEntity *ent,PhysObj *phys,pragma::physics::ICollisionObject *col) {
		auto *convexResult = static_cast<btCollisionWorld::LocalConvexResult*>(ptrFilter->GetUserData());
		return filter(ent,phys,col,*convexResult);
	});
}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<EntityHandle> &filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterEntity>(filter,flags,group,mask);
}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const EntityHandle &filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterEntity>(filter,flags,group,mask);
}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<PhysObjHandle> &filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterPhysObj>(filter,flags,group,mask);
}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const PhysObjHandle &filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterPhysObj>(filter,flags,group,mask);
}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<pragma::physics::ICollisionObject*> &filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterCollisionObject>(filter,flags,group,mask);
}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,pragma::physics::ICollisionObject *filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterCollisionObject>(filter,flags,group,mask);
}
template<class T>
	PhysBaseRayResultCallback<T>::PhysBaseRayResultCallback(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const LuaFunction &filter)
		: PhysBaseRayResultCallback(rayFromWorld,rayToWorld)
{
	m_filter = std::make_shared<BasePhysRayCallbackFilterLuaFunction>(filter,flags,group,mask);
}

//////////////////////////

#define DECLARE_RESULT_CALLBACK(TCALLBACK,TBASE) \
public: \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,nullptr_t); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*)> &filter); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*,const btCollisionWorld::LocalRayResult&)> &filter); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*,const btCollisionWorld::LocalConvexResult&)> &filter); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<EntityHandle> &filter); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const EntityHandle &filter); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<PhysObjHandle> &filter); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const PhysObjHandle &filter); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<pragma::physics::ICollisionObject*> &filter); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,pragma::physics::ICollisionObject *filter); \
	TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const LuaFunction &filter);

class DLLNETWORK PhysClosestRayResultCallback
	: public PhysBaseRayResultCallback<btCollisionWorld::ClosestRayResultCallback>
{
public:
	DECLARE_RESULT_CALLBACK(PhysClosestRayResultCallback,btCollisionWorld::ClosestRayResultCallback);
	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult,bool normalInWorldSpace) override;
	//virtual bool needsCollision(btBroadphaseProxy* proxy0) const override;
};

class DLLNETWORK PhysAllHitsRayResultCallback
	: public PhysBaseRayResultCallback<btCollisionWorld::AllHitsRayResultCallback>
{
public:
	DECLARE_RESULT_CALLBACK(PhysAllHitsRayResultCallback,btCollisionWorld::ClosestRayResultCallback);
	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult,bool normalInWorldSpace) override;
};

class DLLNETWORK PhysClosestConvexResultCallback
	: public PhysBaseRayResultCallback<btCollisionWorld::ClosestConvexResultCallback>
{
public:
	DECLARE_RESULT_CALLBACK(PhysClosestConvexResultCallback,btCollisionWorld::ClosestConvexResultCallback);
	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult &convexResult,bool normalInWorldSpace) override;
};

#endif
#endif

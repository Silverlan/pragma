#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/entities/components/base_physics_component.hpp"

CollisionMask get_collision_group(CollisionMask group,CollisionMask mask)
{
	if(group == CollisionMask::Default)
		return mask;
	return group;
}

BasePhysRayCallbackFilter::BasePhysRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask)
	: m_flags(flags)
{
	if((UInt32(flags) &UInt32(RayCastFlags::InvertFilter)) != 0 && mask == CollisionMask::All)
		mask = CollisionMask::None;
	m_filterGroup = get_collision_group(group,mask);
	m_filterMask = mask;
}

void BasePhysRayCallbackFilter::SetUserData(void *userData) const {m_userData = userData;}
void *BasePhysRayCallbackFilter::GetUserData() const {return m_userData;}

bool BasePhysRayCallbackFilter::ShouldPass(BaseEntity*,PhysObj *phys,pragma::physics::ICollisionObject *physCol)
{
	if(phys == nullptr)
		return true;
	if(
		(umath::to_integral(m_flags) &umath::to_integral(RayCastFlags::IgnoreDynamic)) != 0 && !phys->IsStatic() ||
		(umath::to_integral(m_flags) &umath::to_integral(RayCastFlags::IgnoreStatic)) != 0 && phys->IsStatic()
	)
		return false;
	if(physCol == nullptr)
		return true;
	if((umath::to_integral(m_filterMask) &umath::to_integral(physCol->GetCollisionFilterGroup())) == 0)
		return TranslateFilterValue(false);
	return true;
}

bool BasePhysRayCallbackFilter::TranslateFilterValue(bool b) const
{
	return ((UInt32(m_flags) &UInt32(RayCastFlags::InvertFilter)) == 0) ? b : !b;
}

#define DEFINE_RESULT_CALLBACK(TCALLBACK,TBASE) \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE,CollisionMask,CollisionMask) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld) \
	{ \
		m_collisionFilterGroup = -1; \
		m_collisionFilterMask = -1; \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,nullptr_t) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,nullptr) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*)> &filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,filter) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*,const btCollisionWorld::LocalRayResult&)> &filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,nullptr) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
		m_filter = std::make_shared<BasePhysRayCallbackFilterFunction>(nullptr,flags,group,mask); \
		auto *ptrFilter = static_cast<BasePhysRayCallbackFilterFunction*>(m_filter.get()); \
		ptrFilter->SetFilter([filter,ptrFilter](BaseEntity *ent,PhysObj *phys,PhysCollisionObject *col) { \
			auto *rayResult = static_cast<btCollisionWorld::LocalRayResult*>(ptrFilter->GetUserData()); \
			return filter(ent,phys,col,*rayResult); \
		}); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,PhysCollisionObject*,const btCollisionWorld::LocalConvexResult&)> &filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,nullptr) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
		m_filter = std::make_shared<BasePhysRayCallbackFilterFunction>(nullptr,flags,group,mask); \
		auto *ptrFilter = static_cast<BasePhysRayCallbackFilterFunction*>(m_filter.get()); \
		ptrFilter->SetFilter([filter,ptrFilter](BaseEntity *ent,PhysObj *phys,PhysCollisionObject *col) { \
			auto *convexResult = static_cast<btCollisionWorld::LocalConvexResult*>(ptrFilter->GetUserData()); \
			return filter(ent,phys,col,*convexResult); \
		}); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<EntityHandle> &filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,filter) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const EntityHandle &filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,filter) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<PhysObjHandle> &filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,filter) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const PhysObjHandle &filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,filter) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<PhysCollisionObject*> &filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,filter) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,PhysCollisionObject *filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,filter) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
	} \
	TCALLBACK::TCALLBACK(const btVector3 &rayFromWorld,const btVector3 &rayToWorld,FTRACE flags,CollisionMask group,CollisionMask mask,const LuaFunction &filter) \
		: PhysBaseRayResultCallback<TBASE>(rayFromWorld,rayToWorld,flags,group,mask,filter) \
	{ \
		m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask))); \
		m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask)); \
	}

#ifdef ENABLE_DEPRECATED_PHYSICS
DEFINE_RESULT_CALLBACK(PhysClosestRayResultCallback,btCollisionWorld::ClosestRayResultCallback);
DEFINE_RESULT_CALLBACK(PhysAllHitsRayResultCallback,btCollisionWorld::AllHitsRayResultCallback);
DEFINE_RESULT_CALLBACK(PhysClosestConvexResultCallback,btCollisionWorld::ClosestConvexResultCallback);

btScalar PhysClosestRayResultCallback::addSingleResult(btCollisionWorld::LocalRayResult &rayResult,bool normalInWorldSpace)
{
	auto *obj = rayResult.m_collisionObject;
	auto *colObj = static_cast<PhysCollisionObject*>(obj->getUserPointer());
	if(colObj != nullptr)
	{
		auto *physObj = static_cast<PhysObj*>(colObj->userData);
		if(physObj != nullptr)
		{
			auto *ent = physObj->GetOwner();
			auto pPhysComponent = (ent != nullptr) ? ent->GetEntity().GetPhysicsComponent() : util::WeakHandle<pragma::BasePhysicsComponent>{};
			if(pPhysComponent.valid() && pPhysComponent->IsRayResultCallbackEnabled() == true)
			{
				if(pPhysComponent->RayResultCallback(static_cast<CollisionMask>(m_collisionFilterGroup),static_cast<CollisionMask>(m_collisionFilterMask),m_rayFromWorld,m_rayToWorld,m_hitNormalWorld,m_hitPointWorld,rayResult) == false)
					return 0.0;
			}
			if(m_filter != nullptr)
			{
				m_filter->SetUserData(&rayResult);
				if(m_filter->ShouldPass(&ent->GetEntity(),physObj,colObj) == false)
					return 0.0;
			}
		}
	}
	return btCollisionWorld::ClosestRayResultCallback::addSingleResult(rayResult,normalInWorldSpace);
}

btScalar PhysAllHitsRayResultCallback::addSingleResult(btCollisionWorld::LocalRayResult &rayResult,bool normalInWorldSpace)
{ \
	auto *obj = rayResult.m_collisionObject;
	auto *colObj = static_cast<PhysCollisionObject*>(obj->getUserPointer());
	if(colObj != nullptr)
	{
		auto *physObj = static_cast<PhysObj*>(colObj->userData);
		if(physObj != nullptr)
		{
			auto *ent = physObj->GetOwner();
			auto pPhysComponent = (ent != nullptr) ? ent->GetEntity().GetPhysicsComponent() : util::WeakHandle<pragma::BasePhysicsComponent>{};
			if(pPhysComponent.valid() && pPhysComponent->IsRayResultCallbackEnabled() == true)
			{
				if(pPhysComponent->RayResultCallback(static_cast<CollisionMask>(m_collisionFilterGroup),static_cast<CollisionMask>(m_collisionFilterMask),m_rayFromWorld,m_rayToWorld,m_hitNormalWorld[0],m_hitPointWorld[0],rayResult) == false)
					return 0.0;
			}
			if(m_filter != nullptr)
			{
				m_filter->SetUserData(&rayResult);
				if(m_filter->ShouldPass(&ent->GetEntity(),physObj,colObj) == false)
					return 0.0;
			}
		}
	}
	return btCollisionWorld::AllHitsRayResultCallback::addSingleResult(rayResult,normalInWorldSpace);
}
/*
bool PhysClosestRayResultCallback::needsCollision(btBroadphaseProxy *proxy0) const
{
	btCollisionObject *obj = (btCollisionObject*)proxy0->m_clientObject;
	if(obj != nullptr)
	{
		auto *colObj = static_cast<PhysCollisionObject*>(obj->getUserPointer());
		if(colObj != nullptr)
		{
			auto *physObj = static_cast<PhysObj*>(colObj->userData);
			if(physObj != nullptr)
			{
				BaseEntity *ent = physObj->GetOwner();
				if(ent != nullptr)
				{
					Con::cerr<<"needsCollision: "<<ent->GetClass()<<Con::endl;
				}
			}
		}
	}
	//if(collisionObject == m_collisionObject)
	//	return true;


	bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
	collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
	return collides;
}
*/
btScalar PhysClosestConvexResultCallback::addSingleResult(btCollisionWorld::LocalConvexResult &convexResult,bool normalInWorldSpace)
{
	if(m_filter != nullptr)
	{
		auto *obj = convexResult.m_hitCollisionObject;
		auto *colObj = static_cast<PhysCollisionObject*>(obj->getUserPointer());
		if(colObj != nullptr)
		{
			auto *physObj = static_cast<PhysObj*>(colObj->userData);
			if(physObj != nullptr)
			{
				auto *ent = physObj->GetOwner();
				m_filter->SetUserData(&convexResult);
				if(m_filter->ShouldPass(&ent->GetEntity(),physObj,colObj) == false)
					return 0.0;
			}
		}
	}
	return btCollisionWorld::ClosestConvexResultCallback::addSingleResult(convexResult,normalInWorldSpace);
}
#endif

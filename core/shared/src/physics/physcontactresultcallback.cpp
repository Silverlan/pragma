#ifdef ENABLE_DEPRECATED_PHYSICS

#include "stdafx_shared.h"
#include "pragma/physics/raycallback/physraycallbackfilter_function.hpp"
#include "pragma/physics/raycallback/physraycallbackfilter_entity.hpp"
#include "pragma/physics/raycallback/physraycallbackfilter_physobj.hpp"
#include "pragma/physics/raycallback/physraycallbackfilter_collisionobject.hpp"
#include "pragma/physics/raycallback/physraycallbackfilter_luafunction.hpp"
#include "pragma/physics/raycallback/physraycallback_contact.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/entities/components/base_physics_component.hpp"

PhysContactResultCallback::PhysContactResultCallback(FTRACE,CollisionMask,CollisionMask)
	: btCollisionWorld::ContactResultCallback(),m_filter(nullptr),m_colObj(nullptr),hasHit(false)
{
	m_collisionFilterGroup = -1;
	m_collisionFilterMask = -1;
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,nullptr_t)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*)> &filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = std::make_shared<BasePhysRayCallbackFilterFunction>(filter,flags,group,mask);
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*,const btCollisionWorld::LocalRayResult&)> &filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = nullptr;
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::function<bool(BaseEntity*,PhysObj*,pragma::physics::ICollisionObject*,const btCollisionWorld::LocalConvexResult&)> &filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = nullptr;
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<EntityHandle> &filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = std::make_shared<BasePhysRayCallbackFilterEntity>(filter,flags,group,mask);
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const EntityHandle &filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = std::make_shared<BasePhysRayCallbackFilterEntity>(filter,flags,group,mask);
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<PhysObjHandle> &filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = std::make_shared<BasePhysRayCallbackFilterPhysObj>(filter,flags,group,mask);
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const PhysObjHandle &filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = std::make_shared<BasePhysRayCallbackFilterPhysObj>(filter,flags,group,mask);
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const std::vector<pragma::physics::ICollisionObject*> &filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = std::make_shared<BasePhysRayCallbackFilterCollisionObject>(filter,flags,group,mask);
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,pragma::physics::ICollisionObject *filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = std::make_shared<BasePhysRayCallbackFilterCollisionObject>(filter,flags,group,mask);
}
PhysContactResultCallback::PhysContactResultCallback(FTRACE flags,CollisionMask group,CollisionMask mask,const LuaFunction &filter)
	: PhysContactResultCallback(flags,group,mask)
{
	m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(get_collision_group(group,mask)));
	m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
	m_filter = std::make_shared<BasePhysRayCallbackFilterLuaFunction>(filter,flags,group,mask);
}
btScalar PhysContactResultCallback::addSingleResult(btManifoldPoint &cp,const btCollisionObjectWrapper*,int,int,const btCollisionObjectWrapper *colObj1Wrap,int,int)
{
	auto *obj = colObj1Wrap->getCollisionObject();
	if(m_filter != nullptr)
	{
		auto *colObj = static_cast<pragma::physics::ICollisionObject*>(obj->getUserPointer());
		if(colObj != nullptr)
		{
			auto *physObj = static_cast<PhysObj*>(colObj->userData);
			if(physObj != nullptr)
			{
				auto *ent = physObj->GetOwner();
				if(m_filter->ShouldPass(&ent->GetEntity(),physObj,colObj) == false)
					return 0.0;
			}
		}
	}
	hasHit = true;
	m_positionWorldOnA = uvec::create(cp.m_positionWorldOnA /PhysEnv::WORLD_SCALE);
	m_positionWorldOnB = uvec::create(cp.m_positionWorldOnB /PhysEnv::WORLD_SCALE);
	m_localPointA = uvec::create(cp.m_localPointA /PhysEnv::WORLD_SCALE);
	m_localPointB = uvec::create(cp.m_localPointB /PhysEnv::WORLD_SCALE);
	m_colObj = obj;
	return 0.0;
}
#endif

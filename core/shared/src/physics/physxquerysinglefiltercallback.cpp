#include "stdafx_shared.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/physics/physxquerysinglefiltercallback.h"

PhysXQuerySingleFilterCallback::PhysXQuerySingleFilterCallback(physx::PxRigidActor *filter,physx::PxQueryHitType::Enum e)
	: physx::PxQueryFilterCallback(),m_filter(filter),m_hitType(e)
{}
physx::PxQueryHitType::Enum PhysXQuerySingleFilterCallback::preFilter(const physx::PxFilterData &filterData,const physx::PxShape *shape,const physx::PxRigidActor *actor,physx::PxHitFlags &queryFlags)
{
	if(m_filter == actor)
		return physx::PxQueryHitType::Enum::eNONE;
	return m_hitType;
}
physx::PxQueryHitType::Enum PhysXQuerySingleFilterCallback::postFilter(const physx::PxFilterData &filterData,const physx::PxQueryHit &hit)
{
	return m_hitType;
}
#endif
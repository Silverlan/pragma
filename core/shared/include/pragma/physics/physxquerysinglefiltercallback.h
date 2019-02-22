#ifndef __PHYSXQUERYFILTERSIGNLECALLBACK_H__
#define __PHYSXQUERYFILTERSIGNLECALLBACK_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/networkdefinitions.h"
#include "pragma/physics/physxapi.h"

class DLLNETWORK PhysXQuerySingleFilterCallback
	: public physx::PxQueryFilterCallback
{
protected:
	physx::PxRigidActor *m_filter;
	physx::PxQueryHitType::Enum m_hitType;
public:
	PhysXQuerySingleFilterCallback(physx::PxRigidActor *filter,physx::PxQueryHitType::Enum e=physx::PxQueryHitType::Enum::eTOUCH);
	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData,const physx::PxShape *shape,const physx::PxRigidActor *actor,physx::PxHitFlags &queryFlags);
	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData,const physx::PxQueryHit &hit);
};
#endif

#endif
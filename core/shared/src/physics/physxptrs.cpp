#include "stdafx_shared.h"
#ifdef PHYS_ENGINE_PHYSX
#include "physxptrs.h"

DEFINE_SIMPLE_BASE_HANDLE(DLLNETWORK,physx::PxMaterial,PhysXMaterial);
DEFINE_SIMPLE_BASE_HANDLE(DLLNETWORK,physx::PxActor,PhysXActor);
DEFINE_SIMPLE_DERIVED_HANDLE(DLLNETWORK,PhysXActor,physx::PxRigidActor,PhysXRigidActor);
DEFINE_SIMPLE_DERIVED_HANDLE(DLLNETWORK,PhysXRigidActor,physx::PxRigidDynamic,PhysXRigidDynamic);
DEFINE_SIMPLE_BASE_HANDLE(DLLNETWORK,physx::PxController,PhysXController);
DEFINE_SIMPLE_BASE_HANDLE(DLLNETWORK,physx::PxScene,PhysXScene);

DEFINE_SIMPLE_BASE_HANDLE(DLLNETWORK,physx::PxJoint,PhysXJoint);
DEFINE_SIMPLE_DERIVED_HANDLE(DLLNETWORK,PhysXJoint,physx::PxFixedJoint,PhysXFixedJoint);
DEFINE_SIMPLE_DERIVED_HANDLE(DLLNETWORK,PhysXJoint,physx::PxSphericalJoint,PhysXSphericalJoint);
DEFINE_SIMPLE_DERIVED_HANDLE(DLLNETWORK,PhysXJoint,physx::PxRevoluteJoint,PhysXRevoluteJoint);
DEFINE_SIMPLE_DERIVED_HANDLE(DLLNETWORK,PhysXJoint,physx::PxPrismaticJoint,PhysXPrismaticJoint);
DEFINE_SIMPLE_DERIVED_HANDLE(DLLNETWORK,PhysXJoint,physx::PxDistanceJoint,PhysXDistanceJoint);

DEFINE_SIMPLE_BASE_HANDLE(DLLNETWORK,physx::PxShape,PhysXShape);
#endif
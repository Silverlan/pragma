#ifndef __LDEF_PHYSXMATERIAL_H__
#define __LDEF_PHYSXMATERIAL_H__
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/lua/ldefinitions.h"
#include "pragma/physics/physxapi.h"
#include "physxptrs.h"
lua_registercheck(PhysXMaterial,PhysXMaterial);
lua_registercheck(PhysXActor,PhysXActor);
lua_registercheck(PhysXRigidActor,PhysXRigidActor);
lua_registercheck(PhysXRigidDynamic,PhysXRigidDynamic);
lua_registercheck(PhysXController,PhysXController);
lua_registercheck(PhysXScene,PhysXScene);

lua_registercheck(PhysXJoint,PhysXJoint);
lua_registercheck(PhysXFixedJoint,PhysXFixedJoint);
lua_registercheck(PhysXSphericalJoint,PhysXSphericalJoint);
lua_registercheck(PhysXRevoluteJoint,PhysXRevoluteJoint);
lua_registercheck(PhysXPrismaticJoint,PhysXPrismaticJoint);
lua_registercheck(PhysXDistanceJoint,PhysXDistanceJoint);

lua_registercheck(PhysXShape,PhysXShape);
#endif
#endif
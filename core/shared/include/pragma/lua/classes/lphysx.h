#ifndef __LPHYSX_H__
#define __LPHYSX_H__
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/networkdefinitions.h"
#include <pragma/definitions.h>
#include <pragma/lua/luaapi.h>
#include <mathutil/glmutil.h>
DLLNETWORK int Lua_physx_CreateBoxController(lua_State *l);
DLLNETWORK int Lua_physx_CreateCapsuleController(lua_State *l);
DLLNETWORK int Lua_physx_CreateMaterial(lua_State *l);
DLLNETWORK int Lua_physx_CreateScene(lua_State *l);
DLLNETWORK int Lua_physx_CreateFixedJoint(lua_State *l);
DLLNETWORK int Lua_physx_CreateSphericalJoint(lua_State *l);
DLLNETWORK int Lua_physx_CreateRevoluteJoint(lua_State *l);
DLLNETWORK int Lua_physx_CreatePrismaticJoint(lua_State *l);
DLLNETWORK int Lua_physx_CreateDistanceJoint(lua_State *l);

class PhysXMaterial;
DLLNETWORK void Lua_PhysXMaterial_Release(lua_State *l,PhysXMaterial &mat);

class PhysXJoint;
DLLNETWORK void Lua_PhysXJoint_Release(lua_State *l,PhysXJoint &joint);

class PhysXSphericalJoint;
DLLNETWORK void Lua_PhysXSpericalJoint_SetLimitCone(lua_State *l,PhysXSphericalJoint &joint,float yLimitAngle,float xLimitAngle,float contactDistance=-1);
DLLNETWORK void Lua_PhysXSpericalJoint_EnableLimit(lua_State *l,PhysXSphericalJoint &joint,bool b);

class PhysXActor;
DLLNETWORK void Lua_PhysXActor_Release(lua_State *l,PhysXActor &actor);
DLLNETWORK void Lua_PhysXActor_GetActorFlags(lua_State *l,PhysXActor &actor);
DLLNETWORK void Lua_PhysXActor_SetActorFlags(lua_State *l,PhysXActor &actor,int actorFlags);
DLLNETWORK void Lua_PhysXActor_SetActorFlag(lua_State *l,PhysXActor &actor,int flag,bool b);
DLLNETWORK void Lua_PhysXActor_GetType(lua_State *l,PhysXActor &actor);

class PhysXRigidActor;
class Quat;
DLLNETWORK void Lua_PhysXRigidActor_GetPosition(lua_State *l,PhysXRigidActor &actor);
DLLNETWORK void Lua_PhysXRigidActor_GetOrientation(lua_State *l,PhysXRigidActor &actor);
DLLNETWORK void Lua_PhysXRigidActor_SetPosition(lua_State *l,PhysXRigidActor &actor,Vector3 *pos);
DLLNETWORK void Lua_PhysXRigidActor_SetOrientation(lua_State *l,PhysXRigidActor &actor,Quat *orientation);

class PhysXRigidDynamic;
DLLNETWORK void Lua_PhysXRigidDynamic_AddForce(lua_State *l,PhysXRigidDynamic &actor,Vector3 *force,int mode,bool autowake=false);
DLLNETWORK void Lua_PhysXRigidDynamic_AddTorque(lua_State *l,PhysXRigidDynamic &actor,Vector3 *torque,int mode,bool autowake=false);
DLLNETWORK void Lua_PhysXRigidDynamic_ClearForce(lua_State *l,PhysXRigidDynamic &actor,int mode);
DLLNETWORK void Lua_PhysXRigidDynamic_ClearTorque(lua_State *l,PhysXRigidDynamic &actor,int mode);
DLLNETWORK void Lua_PhysXRigidDynamic_GetAngularDamping(lua_State *l,PhysXRigidDynamic &actor);
DLLNETWORK void Lua_PhysXRigidDynamic_GetAngularVelocity(lua_State *l,PhysXRigidDynamic &actor);
DLLNETWORK void Lua_PhysXRigidDynamic_GetLinearDamping(lua_State *l,PhysXRigidDynamic &actor);
DLLNETWORK void Lua_PhysXRigidDynamic_GetLinearVelocity(lua_State *l,PhysXRigidDynamic &actor);
DLLNETWORK void Lua_PhysXRigidDynamic_GetMass(lua_State *l,PhysXRigidDynamic &actor);
DLLNETWORK void Lua_PhysXRigidDynamic_GetMassSpaceInertiaTensor(lua_State *l,PhysXRigidDynamic &actor);
DLLNETWORK void Lua_PhysXRigidDynamic_GetMaxAngularVelocity(lua_State *l,PhysXRigidDynamic &actor);
DLLNETWORK void Lua_PhysXRigidDynamic_SetAngularDamping(lua_State *l,PhysXRigidDynamic &actor,float angDamp);
DLLNETWORK void Lua_PhysXRigidDynamic_SetAngularVelocity(lua_State *l,PhysXRigidDynamic &actor,Vector3 *angVel);
DLLNETWORK void Lua_PhysXRigidDynamic_SetLinearDamping(lua_State *l,PhysXRigidDynamic &actor,float damping);
DLLNETWORK void Lua_PhysXRigidDynamic_SetLinearVelocity(lua_State *l,PhysXRigidDynamic &actor,Vector3 *vel);
DLLNETWORK void Lua_PhysXRigidDynamic_SetMass(lua_State *l,PhysXRigidDynamic &actor,float mass);
DLLNETWORK void Lua_PhysXRigidDynamic_SetMassSpaceInertiaTensor(lua_State *l,PhysXRigidDynamic &actor,Vector3 *tensor);
DLLNETWORK void Lua_PhysXRigidDynamic_SetMaxAngularVelocity(lua_State *l,PhysXRigidDynamic &actor,float angVel);
DLLNETWORK void Lua_PhysXRigidDynamic_SetMassAndUpdateInertia(lua_State *l,PhysXRigidDynamic &actor,float mass);

class PhysXController;
DLLNETWORK void Lua_PhysXController_Release(lua_State *l,PhysXController &controller);
DLLNETWORK void Lua_PhysXController_Move(lua_State *l,PhysXController &controller,Vector3 *disp,float minDist,float elapsedTime);
DLLNETWORK void Lua_PhysXController_GetPosition(lua_State *l,PhysXController &controller);
DLLNETWORK void Lua_PhysXController_GetFootPosition(lua_State *l,PhysXController &controller);
DLLNETWORK void Lua_PhysXController_GetContactOffset(lua_State *l,PhysXController &controller);
DLLNETWORK void Lua_PhysXController_GetNonWalkableMode(lua_State *l,PhysXController &controller);
DLLNETWORK void Lua_PhysXController_GetSlopeLimit(lua_State *l,PhysXController &controller);
DLLNETWORK void Lua_PhysXController_GetStepOffset(lua_State *l,PhysXController &controller);
DLLNETWORK void Lua_PhysXController_GetUpDirection(lua_State *l,PhysXController &controller);
DLLNETWORK void Lua_PhysXController_GetActor(lua_State *l,PhysXController &controller);
DLLNETWORK void Lua_PhysXController_SetPosition(lua_State *l,PhysXController &controller,Vector3 *pos);
DLLNETWORK void Lua_PhysXController_SetFootPosition(lua_State *l,PhysXController &controller,Vector3 *pos);
DLLNETWORK void Lua_PhysXController_SetContactOffset(lua_State *l,PhysXController &controller,float contactOffset);
DLLNETWORK void Lua_PhysXController_SetNonWalkableMode(lua_State *l,PhysXController &controller,int nonWalkableMode);
DLLNETWORK void Lua_PhysXController_SetSlopeLimit(lua_State *l,PhysXController &controller,float slopeLimit);
DLLNETWORK void Lua_PhysXController_SetStepOffset(lua_State *l,PhysXController &controller,float stepOffset);
DLLNETWORK void Lua_PhysXController_SetUpDirection(lua_State *l,PhysXController &controller,Vector3 *up);

class PhysXScene;
DLLNETWORK void Lua_PhysXScene_ShiftOrigin(lua_State *l,PhysXScene &scene,Vector3 *shift);
DLLNETWORK void Lua_PhysXScene_Release(lua_State *l,PhysXScene &scene);
DLLNETWORK void Lua_PhysXScene_SetFlag(lua_State *l,PhysXScene &scene,int flag,bool b);
DLLNETWORK void Lua_PhysXScene_SetGravity(lua_State *l,PhysXScene &scene,Vector3 &gravity);
DLLNETWORK void Lua_PhysXScene_GetGravity(lua_State *l,PhysXScene &scene);
DLLNETWORK void Lua_PhysXScene_Simulate(lua_State *l,PhysXScene &scene,float elapsedTime);
DLLNETWORK void Lua_PhysXScene_FetchResults(lua_State *l,PhysXScene &scene,bool block=false);
DLLNETWORK void Lua_PhysXScene_GetActors(lua_State *l,PhysXScene &scene,int types);
DLLNETWORK void Lua_PhysXScene_GetActors(lua_State *l,PhysXScene &scene);
DLLNETWORK void Lua_PhysXScene_AddActor(lua_State *l,PhysXScene &scene,PhysXActor &actor);
DLLNETWORK void Lua_PhysXScene_RemoveActor(lua_State *l,PhysXScene &scene,PhysXActor &actor);
DLLNETWORK void Lua_PhysXScene_RayCast(lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,float distance,unsigned int flags,bool bAnyHit);
DLLNETWORK void Lua_PhysXScene_RayCast(lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,float distance,unsigned int flags);
DLLNETWORK void Lua_PhysXScene_RayCast(lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,float distance);
DLLNETWORK void Lua_PhysXScene_RayCast(
		lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,
		float distance,unsigned int flags,unsigned int numHits
	);
DLLNETWORK void Lua_PhysXScene_RayCast(
		lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,
		float distance,unsigned int flags,luabind::object oCallback
	);
#endif
#endif

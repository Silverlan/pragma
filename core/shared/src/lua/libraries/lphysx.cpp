#include "stdafx_shared.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/lua/classes/lphysx.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/classes/ldef_vector.h"
#include <pragma/engine.h>
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "luasystem.h"
#include "pragma/physics/physxapi.h"
#include "pragma/lua/classes/ldef_physx.h"
#include "pragma/lua/classes/ldef_quaternion.h"

extern DLLENGINE Engine *engine;
DLLNETWORK int Lua_physx_CreateBoxController(lua_State *l)
{
	Lua::CheckTable(l,1);
	physx::PxBoxControllerDesc desc;
	Lua::PushValue(l,1);
	int table = Lua::GetStackTop(l);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,table) != 0)
	{
		Lua::PushValue(l,-2);
		std::string key = Lua::ToString(l,-3);
		std::transform(key.begin(),key.end(),key.begin(),::tolower);
		Lua::RemoveValue(l,-3);
		if(key == "contactoffset")
			desc.contactOffset = Lua::ToNumber(l,-2);
		else if(key == "density")
			desc.density = Lua::ToNumber(l,-2);
		else if(key == "halfforwardextent")
			desc.halfForwardExtent = Lua::ToNumber(l,-2);
		else if(key == "halfheight")
			desc.halfHeight = Lua::ToNumber(l,-2);
		else if(key == "halfsideextent")
			desc.halfSideExtent = Lua::ToNumber(l,-2);
		else if(key == "material")
		{
			if(_lua_isPhysXMaterial(l,-2))
				desc.material = _lua_PhysXMaterial_get(l,-2)->get();
		}
		else if(key == "nonwalkablemode")
			desc.nonWalkableMode = physx::PxControllerNonWalkableMode::Enum(Lua::ToInt(l,-2));
		else if(key == "position")
		{
			if(_lua_isVector(l,-2))
			{
				Vector3 *pos = _lua_Vector_get(l,-2);
				desc.position = physx::PxExtendedVec3(pos->x,pos->y,pos->z);
			}
		}
		else if(key == "scalecoeff")
			desc.scaleCoeff = Lua::ToNumber(l,-2);
		else if(key == "slopelimit")
			desc.slopeLimit = Lua::ToNumber(l,-2);
		else if(key == "stepoffset")
			desc.stepOffset = Lua::ToNumber(l,-2);
		else if(key == "updirection")
		{
			if(_lua_isVector(l,-2))
			{
				Vector3 *pos = _lua_Vector_get(l,-2);
				desc.upDirection = physx::PxVec3(pos->x,pos->y,pos->z);
			}
		}
	}
	Lua::Pop(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	physx::PxControllerManager *manager = game->GetPhysXControllerManager();
	physx::PxController *controller = manager->createController(desc);
	if(controller == NULL)
		return 0;
	Lua::Push<PhysXController>(l,PhysXController(controller));
	return 1;
}

DLLNETWORK int Lua_physx_CreateCapsuleController(lua_State *l)
{
	Lua::CheckTable(l,1);
	physx::PxCapsuleControllerDesc desc;
	Lua::PushValue(l,1);
	int table = Lua::GetStackTop(l);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,table) != 0)
	{
		Lua::PushValue(l,-2);
		std::string key = Lua::ToString(l,-3);
		std::transform(key.begin(),key.end(),key.begin(),::tolower);
		Lua::RemoveValue(l,-3);
		if(key == "climbingmode")
			desc.climbingMode = physx::PxCapsuleClimbingMode::Enum(Lua::ToInt(l,-2));
		else if(key == "contactoffset")
			desc.contactOffset = Lua::ToNumber(l,-2);
		else if(key == "density")
			desc.density = Lua::ToNumber(l,-2);
		else if(key == "height")
			desc.height = Lua::ToNumber(l,-2);
		else if(key == "material")
		{
			if(_lua_isPhysXMaterial(l,-2))
				desc.material = _lua_PhysXMaterial_get(l,-2)->get();
		}
		else if(key == "nonwalkablemode")
			desc.nonWalkableMode = physx::PxControllerNonWalkableMode::Enum(Lua::ToInt(l,-2));
		else if(key == "position")
		{
			if(_lua_isVector(l,-2))
			{
				Vector3 *pos = _lua_Vector_get(l,-2);
				desc.position = physx::PxExtendedVec3(pos->x,pos->y,pos->z);
			}
		}
		else if(key == "radius")
			desc.radius = Lua::ToNumber(l,-2);
		else if(key == "scalecoeff")
			desc.scaleCoeff = Lua::ToNumber(l,-2);
		else if(key == "slopelimit")
			desc.slopeLimit = Lua::ToNumber(l,-2);
		else if(key == "stepoffset")
			desc.stepOffset = Lua::ToNumber(l,-2);
		else if(key == "updirection")
		{
			if(_lua_isVector(l,-2))
			{
				Vector3 *pos = _lua_Vector_get(l,-2);
				desc.upDirection = physx::PxVec3(pos->x,pos->y,pos->z);
			}
		}
	}
	Lua::Pop(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	physx::PxControllerManager *manager = game->GetPhysXControllerManager();
	physx::PxController *controller = manager->createController(desc);
	if(controller == NULL)
		return 0;
	Lua::Push<PhysXController>(l,PhysXController(controller));
	return 1;
}

DLLNETWORK int Lua_physx_CreateMaterial(lua_State *l)
{
	float staticFriction = luaL_checknumber(l,1);
	float dynamicFriction = luaL_checknumber(l,2);
	float restitution = luaL_checknumber(l,3);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();

	physx::PxPhysics *physics = state->GetPhysics();
	physx::PxMaterial *material = physics->createMaterial(staticFriction,dynamicFriction,restitution);
	if(material == NULL)
		return 0;
	Lua::Push<PhysXMaterial>(l,PhysXMaterial(material));
	return 1;
}

static physx::PxFilterFlags CCDFilterShader(
	physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags,
	const void* constantBlock,
	physx::PxU32 constantBlockSize
)
{
	pairFlags = physx::PxPairFlag::eRESOLVE_CONTACTS;
	pairFlags |= physx::PxPairFlag::eCCD_LINEAR;
	return physx::PxFilterFlags();
}

DLLNETWORK int Lua_physx_CreateScene(lua_State *l)
{
	physx::PxDefaultCpuDispatcher *dispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	if(dispatcher == NULL)
		return 0;
	NetworkState *state = engine->GetNetworkState(l);
	physx::PxPhysics *physics = state->GetPhysics();
	physx::PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = CCDFilterShader;

	Lua::CheckTable(l,1);
	Lua::PushValue(l,1);
	int table = Lua::GetStackTop(l);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,table) != 0)
	{
		Lua::PushValue(l,-2);
		std::string key = Lua::ToString(l,-3);
		std::transform(key.begin(),key.end(),key.begin(),::tolower);
		Lua::RemoveValue(l,-3);
		if(key == "gravity")
		{
			Vector3 *gravity = Lua::CheckVector(l,-2);
			sceneDesc.gravity = physx::PxVec3(gravity->x,gravity->y,gravity->z);
		}
		else if(key == "flags")
			sceneDesc.flags = physx::PxSceneFlags(Lua::ToInt(l,-2));
		else if(key == "bounceThresholdVelocity")
			sceneDesc.bounceThresholdVelocity = Lua::ToNumber(l,-2);
	}
	Lua::Pop(l,1);
	if(!sceneDesc.isValid())
		return 0;
	physx::PxScene *scene = physics->createScene(sceneDesc);
	if(scene == NULL)
		return 0;
	Lua::Push<PhysXScene>(l,PhysXScene(scene));
	return 1;
}

template<class TPhysXJoint,class TLuaPhysXJoint>
	static int CreatePhysXJoint(lua_State *l,TPhysXJoint*(*createJoint)(physx::PxPhysics&,physx::PxRigidActor*,const physx::PxTransform&,physx::PxRigidActor*,const physx::PxTransform&))
{
	physx::PxRigidActor *actorA = Lua::CheckPhysXRigidActor(l,1)->get();
	Vector3 *posA = Lua::CheckVector(l,2);
	auto *orientationA = Lua::CheckQuaternion(l,3);
	physx::PxTransform tA(
		physx::PxVec3(posA->x,posA->y,posA->z),
		physx::PxQuat(orientationA->x,orientationA->y,orientationA->z,orientationA->w)
	);
	physx::PxRigidActor *actorB = Lua::CheckPhysXRigidActor(l,4)->get();
	Vector3 *posB = Lua::CheckVector(l,5);
	auto *orientationB = Lua::CheckQuaternion(l,6);
	physx::PxTransform tB(
		physx::PxVec3(posB->x,posB->y,posB->z),
		physx::PxQuat(orientationB->x,orientationB->y,orientationB->z,orientationB->w)
	);
	NetworkState *state = engine->GetNetworkState(l);
	physx::PxPhysics *physics = state->GetPhysics();
	TPhysXJoint *joint = createJoint(
		*physics,
		actorA,
		tA,
		actorB,
		tB
	);
	if(joint == NULL)
		return 0;
	Lua::Push<TLuaPhysXJoint>(l,TLuaPhysXJoint(joint));
	return 1;
}

DLLNETWORK int Lua_physx_CreateFixedJoint(lua_State *l) {return CreatePhysXJoint<physx::PxFixedJoint,PhysXFixedJoint>(l,physx::PxFixedJointCreate);}
DLLNETWORK int Lua_physx_CreateSphericalJoint(lua_State *l) {return CreatePhysXJoint<physx::PxSphericalJoint,PhysXSphericalJoint>(l,physx::PxSphericalJointCreate);}
DLLNETWORK int Lua_physx_CreateRevoluteJoint(lua_State *l) {return CreatePhysXJoint<physx::PxRevoluteJoint,PhysXRevoluteJoint>(l,physx::PxRevoluteJointCreate);}
DLLNETWORK int Lua_physx_CreatePrismaticJoint(lua_State *l) {return CreatePhysXJoint<physx::PxPrismaticJoint,PhysXPrismaticJoint>(l,physx::PxPrismaticJointCreate);}
DLLNETWORK int Lua_physx_CreateDistanceJoint(lua_State *l) {return CreatePhysXJoint<physx::PxDistanceJoint,PhysXDistanceJoint>(l,physx::PxDistanceJointCreate);}

/////////////////////////////

DLLNETWORK void Lua_PhysXMaterial_Release(lua_State *l,PhysXMaterial &mat)
{
	mat->release();
}

/////////////////////////////

DLLNETWORK void Lua_PhysXJoint_Release(lua_State *l,PhysXJoint &joint)
{
	joint->release();
}

DLLNETWORK void Lua_PhysXSpericalJoint_SetLimitCone(lua_State *l,PhysXSphericalJoint &joint,float yLimitAngle,float xLimitAngle,float contactDistance)
{
	joint->setLimitCone(physx::PxJointLimitCone(yLimitAngle,xLimitAngle,contactDistance));
}

DLLNETWORK void Lua_PhysXSpericalJoint_EnableLimit(lua_State *l,PhysXSphericalJoint &joint,bool b)
{
	joint->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED,b);
}

/////////////////////////////

DLLNETWORK void Lua_PhysXActor_Release(lua_State *l,PhysXActor &actor)
{
	actor->release();
}

DLLNETWORK void Lua_PhysXActor_GetActorFlags(lua_State *l,PhysXActor &actor)
{
	Lua::PushInt(l,physx::PxU16(actor->getActorFlags()));
}

DLLNETWORK void Lua_PhysXActor_SetActorFlags(lua_State *l,PhysXActor &actor,int actorFlags)
{
	actor->setActorFlags(physx::PxActorFlags(actorFlags));
}

DLLNETWORK void Lua_PhysXActor_SetActorFlag(lua_State *l,PhysXActor &actor,int flag,bool b)
{
	actor->setActorFlag(physx::PxActorFlag::Enum(flag),b);
}

DLLNETWORK void Lua_PhysXActor_GetType(lua_State *l,PhysXActor &actor)
{
	Lua::PushInt(l,actor->getType());
}

/////////////////////////////

DLLNETWORK void Lua_PhysXRigidActor_GetPosition(lua_State *l,PhysXRigidActor &actor)
{
	physx::PxTransform t = actor->getGlobalPose();
	Lua::Push<Vector3>(l,Vector3(t.p.x,t.p.y,t.p.z));
}

DLLNETWORK void Lua_PhysXRigidActor_GetOrientation(lua_State *l,PhysXRigidActor &actor)
{
	physx::PxTransform t = actor->getGlobalPose();
	Lua::Push<Quat>(l,Quat(t.q.w,t.q.x,t.q.y,t.q.z));
}

DLLNETWORK void Lua_PhysXRigidActor_SetPosition(lua_State *l,PhysXRigidActor &actor,Vector3 *pos)
{
	physx::PxTransform t = actor->getGlobalPose();
	t.p.x = pos->x;
	t.p.y = pos->y;
	t.p.z = pos->z;
	actor->setGlobalPose(t);
}

DLLNETWORK void Lua_PhysXRigidActor_SetOrientation(lua_State *l,PhysXRigidActor &actor,Quat *orientation)
{
	physx::PxTransform t = actor->getGlobalPose();
	t.q.w = orientation->w;
	t.q.x = orientation->x;
	t.q.y = orientation->y;
	t.q.z = orientation->z;
	actor->setGlobalPose(t);
}

/////////////////////////////

DLLNETWORK void Lua_PhysXRigidDynamic_AddForce(lua_State *l,PhysXRigidDynamic &actor,Vector3 *force,int mode,bool autowake)
{
	actor->addForce(physx::PxVec3(force->x,force->y,force->z),physx::PxForceMode::Enum(mode),autowake);
}

DLLNETWORK void Lua_PhysXRigidDynamic_AddTorque(lua_State *l,PhysXRigidDynamic &actor,Vector3 *torque,int mode,bool autowake)
{
	actor->addTorque(physx::PxVec3(torque->x,torque->y,torque->z),physx::PxForceMode::Enum(mode),autowake);
}

DLLNETWORK void Lua_PhysXRigidDynamic_ClearForce(lua_State *l,PhysXRigidDynamic &actor,int mode)
{
	actor->clearForce(physx::PxForceMode::Enum(mode));
}
DLLNETWORK void Lua_PhysXRigidDynamic_ClearTorque(lua_State *l,PhysXRigidDynamic &actor,int mode)
{
	actor->clearTorque(physx::PxForceMode::Enum(mode));
}
DLLNETWORK void Lua_PhysXRigidDynamic_GetAngularDamping(lua_State *l,PhysXRigidDynamic &actor)
{
	Lua::PushNumber(l,actor->getAngularDamping());
}
DLLNETWORK void Lua_PhysXRigidDynamic_GetAngularVelocity(lua_State *l,PhysXRigidDynamic &actor)
{
	physx::PxVec3 velAng = actor->getAngularVelocity();
	Lua::Push<Vector3>(l,Vector3(velAng.x,velAng.y,velAng.z));
}
DLLNETWORK void Lua_PhysXRigidDynamic_GetLinearDamping(lua_State *l,PhysXRigidDynamic &actor)
{
	Lua::PushNumber(l,actor->getLinearDamping());
}
DLLNETWORK void Lua_PhysXRigidDynamic_GetLinearVelocity(lua_State *l,PhysXRigidDynamic &actor)
{
	physx::PxVec3 vel = actor->getLinearVelocity();
	Lua::Push<Vector3>(l,Vector3(vel.x,vel.y,vel.z));
}
DLLNETWORK void Lua_PhysXRigidDynamic_GetMass(lua_State *l,PhysXRigidDynamic &actor)
{
	Lua::PushNumber(l,actor->getMass());
}
DLLNETWORK void Lua_PhysXRigidDynamic_GetMassSpaceInertiaTensor(lua_State *l,PhysXRigidDynamic &actor)
{
	physx::PxVec3 tensor = actor->getMassSpaceInertiaTensor();
	Lua::Push<Vector3>(l,Vector3(tensor.x,tensor.y,tensor.z));
}
DLLNETWORK void Lua_PhysXRigidDynamic_GetMaxAngularVelocity(lua_State *l,PhysXRigidDynamic &actor)
{
	Lua::PushNumber(l,actor->getMaxAngularVelocity());
}
DLLNETWORK void Lua_PhysXRigidDynamic_SetAngularDamping(lua_State *l,PhysXRigidDynamic &actor,float angDamp)
{
	actor->setAngularDamping(angDamp);
}
DLLNETWORK void Lua_PhysXRigidDynamic_SetAngularVelocity(lua_State *l,PhysXRigidDynamic &actor,Vector3 *angVel)
{
	actor->setAngularVelocity(physx::PxVec3(angVel->x,angVel->y,angVel->z));
}
DLLNETWORK void Lua_PhysXRigidDynamic_SetLinearDamping(lua_State *l,PhysXRigidDynamic &actor,float damping)
{
	actor->setLinearDamping(damping);
}
DLLNETWORK void Lua_PhysXRigidDynamic_SetLinearVelocity(lua_State *l,PhysXRigidDynamic &actor,Vector3 *vel)
{
	actor->setLinearVelocity(physx::PxVec3(vel->x,vel->y,vel->z));
}
DLLNETWORK void Lua_PhysXRigidDynamic_SetMass(lua_State *l,PhysXRigidDynamic &actor,float mass)
{
	actor->setMass(mass);
}
DLLNETWORK void Lua_PhysXRigidDynamic_SetMassSpaceInertiaTensor(lua_State *l,PhysXRigidDynamic &actor,Vector3 *tensor)
{
	actor->setMassSpaceInertiaTensor(physx::PxVec3(tensor->x,tensor->y,tensor->z));
}
DLLNETWORK void Lua_PhysXRigidDynamic_SetMaxAngularVelocity(lua_State *l,PhysXRigidDynamic &actor,float angVel)
{
	actor->setMaxAngularVelocity(angVel);
}

DLLNETWORK void Lua_PhysXRigidDynamic_SetMassAndUpdateInertia(lua_State *l,PhysXRigidDynamic &actor,float mass)
{
	bool b = physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor.get(),mass);
	Lua::PushBool(l,b);
}

/////////////////////////////

DLLNETWORK void Lua_PhysXController_Release(lua_State *l,PhysXController &controller)
{
	controller->release();
}

DLLNETWORK void Lua_PhysXController_Move(lua_State *l,PhysXController &controller,Vector3 *disp,float minDist,float elapsedTime)
{
	physx::PxControllerCollisionFlags flags = controller->move(physx::PxVec3(disp->x,disp->y,disp->z),minDist,elapsedTime,physx::PxControllerFilters());
	Lua::PushInt(l,unsigned int(flags));
}

DLLNETWORK void Lua_PhysXController_GetPosition(lua_State *l,PhysXController &controller)
{
	physx::PxExtendedVec3 pos = controller->getPosition();
	Lua::Push<Vector3>(l,Vector3(pos.x,pos.y,pos.z));
}

DLLNETWORK void Lua_PhysXController_GetFootPosition(lua_State *l,PhysXController &controller)
{
	physx::PxExtendedVec3 pos = controller->getFootPosition();
	Lua::Push<Vector3>(l,Vector3(pos.x,pos.y,pos.z));
}

DLLNETWORK void Lua_PhysXController_GetContactOffset(lua_State *l,PhysXController &controller)
{
	float contactOffset = controller->getContactOffset();
	Lua::PushNumber(l,contactOffset);
}

DLLNETWORK void Lua_PhysXController_GetNonWalkableMode(lua_State *l,PhysXController &controller)
{
	int nonWalkableMode = controller->getNonWalkableMode();
	Lua::PushInt(l,nonWalkableMode);
}

DLLNETWORK void Lua_PhysXController_GetSlopeLimit(lua_State *l,PhysXController &controller)
{
	float slopeLimit = controller->getSlopeLimit();
	Lua::PushNumber(l,slopeLimit);
}

DLLNETWORK void Lua_PhysXController_GetStepOffset(lua_State *l,PhysXController &controller)
{
	float stepOffset = controller->getStepOffset();
	Lua::PushNumber(l,stepOffset);
}

DLLNETWORK void Lua_PhysXController_GetUpDirection(lua_State *l,PhysXController &controller)
{
	physx::PxVec3 pos = controller->getUpDirection();
	Lua::Push<Vector3>(l,Vector3(pos.x,pos.y,pos.z));
}

DLLNETWORK void Lua_PhysXController_GetActor(lua_State *l,PhysXController &controller)
{
	physx::PxActor *actor = controller->getActor();
	if(actor->isRigidDynamic())
		Lua::Push<PhysXRigidDynamic>(l,PhysXRigidDynamic(static_cast<physx::PxRigidDynamic*>(actor)));
	else if(actor->isRigidActor())
		Lua::Push<PhysXRigidActor>(l,PhysXRigidActor(static_cast<physx::PxRigidActor*>(actor)));
	else
		Lua::Push<PhysXActor>(l,PhysXActor(actor));
}

DLLNETWORK void Lua_PhysXController_SetPosition(lua_State *l,PhysXController &controller,Vector3 *pos)
{
	controller->setPosition(physx::PxExtendedVec3(pos->x,pos->y,pos->z));
}

DLLNETWORK void Lua_PhysXController_SetFootPosition(lua_State *l,PhysXController &controller,Vector3 *pos)
{
	controller->setFootPosition(physx::PxExtendedVec3(pos->x,pos->y,pos->z));
}

DLLNETWORK void Lua_PhysXController_SetContactOffset(lua_State *l,PhysXController &controller,float contactOffset)
{
	controller->setContactOffset(contactOffset);
}

DLLNETWORK void Lua_PhysXController_SetNonWalkableMode(lua_State *l,PhysXController &controller,int nonWalkableMode)
{
	controller->setNonWalkableMode(physx::PxControllerNonWalkableMode::Enum(nonWalkableMode));
}

DLLNETWORK void Lua_PhysXController_SetSlopeLimit(lua_State *l,PhysXController &controller,float slopeLimit)
{
	controller->setSlopeLimit(slopeLimit);
}

DLLNETWORK void Lua_PhysXController_SetStepOffset(lua_State *l,PhysXController &controller,float stepOffset)
{
	controller->setStepOffset(stepOffset);
}

DLLNETWORK void Lua_PhysXController_SetUpDirection(lua_State *l,PhysXController &controller,Vector3 *up)
{
	controller->setUpDirection(physx::PxVec3(up->x,up->y,up->z));
}

/////////////////////////////

DLLNETWORK void Lua_PhysXScene_ShiftOrigin(lua_State *l,PhysXScene &scene,Vector3 *shift) {scene->shiftOrigin(physx::PxVec3(shift->x,shift->y,shift->z));}
DLLNETWORK void Lua_PhysXScene_Release(lua_State *l,PhysXScene &scene) {scene->release();}
DLLNETWORK void Lua_PhysXScene_SetFlag(lua_State *l,PhysXScene &scene,int flag,bool b) {scene->setFlag(physx::PxSceneFlag::Enum(flag),b);}
DLLNETWORK void Lua_PhysXScene_SetGravity(lua_State *l,PhysXScene &scene,Vector3 &gravity) {scene->setGravity(physx::PxVec3(gravity.x,gravity.y,gravity.z));}
DLLNETWORK void Lua_PhysXScene_GetGravity(lua_State *l,PhysXScene &scene)
{
	physx::PxVec3 gravity = scene->getGravity();
	Lua::Push<Vector3>(l,Vector3(gravity.x,gravity.y,gravity.z));
}
DLLNETWORK void Lua_PhysXScene_Simulate(lua_State *l,PhysXScene &scene,float elapsedTime) {scene->simulate(elapsedTime);}
DLLNETWORK void Lua_PhysXScene_FetchResults(lua_State *l,PhysXScene &scene,bool block)
{
	bool b = scene->fetchResults(block);
	Lua::PushBool(l,b);
}
static void Lua_PhysXScene_GetActors(lua_State *l,PhysXScene &scene,physx::PxActorTypeFlags types)
{
	unsigned int numActors = scene->getNbActors(physx::PxActorTypeFlags(types));
	physx::PxActor **actors = new physx::PxActor*[numActors];
	scene->getActors(physx::PxActorTypeFlags(types),&actors[0],numActors);
	Lua::CreateTable(l);
	int top = Lua::GetStackTop(l);
	for(unsigned int i=0;i<numActors;i++)
	{
		switch(actors[i]->getType())
		{
		case physx::PxActorType::Enum::eRIGID_DYNAMIC:
			Lua::Push<PhysXActor>(l,PhysXRigidDynamic(static_cast<physx::PxRigidDynamic*>(actors[i])));
		default:
			Lua::Push<PhysXActor>(l,PhysXActor(actors[i]));
		}
		Lua::SetTableValue(l,top,i +1);
	}
	delete[] actors;
}
DLLNETWORK void Lua_PhysXScene_GetActors(lua_State *l,PhysXScene &scene,int types) {Lua_PhysXScene_GetActors(l,scene,physx::PxActorTypeFlags(types));}
DLLNETWORK void Lua_PhysXScene_GetActors(lua_State *l,PhysXScene &scene) {Lua_PhysXScene_GetActors(l,scene,physx::PxActorTypeFlag::eCLOTH | physx::PxActorTypeFlag::ePARTICLE_FLUID | physx::PxActorTypeFlag::ePARTICLE_SYSTEM | physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC);}
DLLNETWORK void Lua_PhysXScene_AddActor(lua_State *l,PhysXScene &scene,PhysXActor &actor) {scene->addActor(*actor.get());}
DLLNETWORK void Lua_PhysXScene_RemoveActor(lua_State *l,PhysXScene &scene,PhysXActor &actor) {scene->removeActor(*actor.get());}

static void WriteBlockData(lua_State *l,int table,const physx::PxRaycastHit *hit)
{
	Lua::PushString(l,"actor");
	Lua::Push<PhysXRigidActor>(l,PhysXRigidActor(hit->actor));
	Lua::SetTableValue(l,table);

	Lua::PushString(l,"distance");
	Lua::PushNumber(l,hit->distance);
	Lua::SetTableValue(l,table);

	Lua::PushString(l,"faceIndex");
	Lua::PushInt(l,hit->faceIndex);
	Lua::SetTableValue(l,table);

	Lua::PushString(l,"flags");
	Lua::PushInt(l,unsigned int(hit->flags));
	Lua::SetTableValue(l,table);

	Lua::PushString(l,"hadInitialOverlap");
	Lua::PushBool(l,hit->hadInitialOverlap());
	Lua::SetTableValue(l,table);

	const physx::PxVec3 &normal = hit->normal;
	Lua::PushString(l,"normal");
	Lua::Push<Vector3>(l,Vector3(normal.x,normal.y,normal.z));
	Lua::SetTableValue(l,table);

	const physx::PxVec3 &position = hit->position;
	Lua::PushString(l,"position");
	Lua::Push<Vector3>(l,Vector3(position.x,position.y,position.z));
	Lua::SetTableValue(l,table);

	Lua::PushString(l,"u");
	Lua::PushNumber(l,hit->u);
	Lua::SetTableValue(l,table);

	Lua::PushString(l,"v");
	Lua::PushNumber(l,hit->v);
	Lua::SetTableValue(l,table);

	Lua::PushString(l,"shape");
	Lua::Push<PhysXShape>(l,PhysXShape(hit->shape));
	Lua::SetTableValue(l,table);
}

DLLNETWORK void Lua_PhysXScene_RayCast(lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,float distance,unsigned int flags,bool bAnyHit)
{
	physx::PxScene *pxScene = scene.get();
	physx::PxVec3 pxOrigin(origin.x,origin.y,origin.z);
	physx::PxVec3 pxDir(unitDir.x,unitDir.y,unitDir.z);
	physx::PxRaycastBuffer hit;
	physx::PxHitFlags pxFlags(flags);
	physx::PxQueryFilterData filterData;
	if(bAnyHit == true)
		filterData.flags |= physx::PxQueryFlag::eANY_HIT;
	
	bool b = (pxScene->raycast(
		pxOrigin,
		pxDir,
		distance,
		hit,
		pxFlags,
		filterData
	)) && hit.hasBlock ? true : false;

	Lua::PushBool(l,b);
	if(bAnyHit == true || b == false)
		return;
	Lua::CreateTable(l);
	int table = Lua::GetStackTop(l);
	WriteBlockData(l,table,&hit.block);
}

DLLNETWORK void Lua_PhysXScene_RayCast(lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,float distance,unsigned int flags)
{
	Lua_PhysXScene_RayCast(l,scene,origin,unitDir,distance,flags,false);
}

DLLNETWORK void Lua_PhysXScene_RayCast(lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,float distance)
{
	Lua_PhysXScene_RayCast(l,scene,origin,unitDir,distance,physx::PxHitFlag::eDEFAULT);
}

DLLNETWORK void Lua_PhysXScene_RayCast(
		lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,
		float distance,unsigned int flags,unsigned int numHits
	)
{
	physx::PxScene *pxScene = scene.get();
	physx::PxVec3 pxOrigin(origin.x,origin.y,origin.z);
	physx::PxVec3 pxDir(unitDir.x,unitDir.y,unitDir.z);
	physx::PxRaycastHit *hitBuffer = new physx::PxRaycastHit[numHits];
	physx::PxRaycastBuffer hit(&hitBuffer[0],numHits);
	physx::PxHitFlags pxFlags(flags);
	
	bool b = pxScene->raycast(
		pxOrigin,
		pxDir,
		distance,
		hit,
		pxFlags
	);
	Lua::PushBool(l,b);
	Lua::CreateTable(l);
	int table = Lua::GetStackTop(l);
	if(b == true)
	{
		Lua::PushString(l,"block");
		Lua::CreateTable(l);
		int block = Lua::GetStackTop(l);
		WriteBlockData(l,block,&hit.block);
		Lua::SetTableValue(l,table);
	}
	Lua::PushString(l,"touches");
	Lua::CreateTable(l);
	int touches = Lua::GetStackTop(l);
	for(unsigned int i=0;i<hit.nbTouches;i++)
	{
		Lua::PushInt(l,i +1);
		Lua::CreateTable(l);
		int touch = Lua::GetStackTop(l);
		WriteBlockData(l,touch,&hit.touches[i]);
		Lua::SetTableValue(l,touches);
	}
	Lua::SetTableValue(l,table);
	delete[] hitBuffer;
}

static struct PxLuaRaycastCallback
	: physx::PxRaycastCallback
{
protected:
	luabind::object m_callback;
	lua_State *m_lua;
public:
	PxLuaRaycastCallback(lua_State *l,luabind::object callback,physx::PxRaycastHit *aTouches,physx::PxU32 aMaxNbTouches)
		: m_lua(l),m_callback(callback),physx::PxRaycastCallback(aTouches,aMaxNbTouches)
	{}
	virtual physx::PxAgain processTouches(const physx::PxRaycastHit *buffer,physx::PxU32 nbHits) override
	{
		Lua::CreateTable(m_lua);
		int table = Lua::GetStackTop(m_lua);
		for(unsigned int i=0;i<nbHits;i++)
		{
			Lua::PushInt(m_lua,i +1);
			Lua::CreateTable(m_lua);
			int touch = Lua::GetStackTop(m_lua);
			WriteBlockData(m_lua,touch,&buffer[i]);
			Lua::SetTableValue(m_lua,table);
		}
		bool bContinue = true;
		int s = Lua::ProtectedCall(m_lua,1,1);
		if(s == 0)
			bContinue = Lua::ToBool(m_lua,-1);
		NetworkState *state = engine->GetNetworkState(m_lua);
		_luaerror(state->GetGameState(),s,m_lua);
		return bContinue;
	}
	virtual void finalizeQuery() override
	{}
};

DLLNETWORK void Lua_PhysXScene_RayCast(
		lua_State *l,PhysXScene &scene,Vector3 &origin,Vector3 &unitDir,
		float distance,unsigned int flags,luabind::object oCallback
	)
{
	NetworkState *state = engine->GetNetworkState(l);
	physx::PxScene *pxScene = scene.get();
	physx::PxVec3 pxOrigin(origin.x,origin.y,origin.z);
	physx::PxVec3 pxDir(unitDir.x,unitDir.y,unitDir.z);
	physx::PxRaycastHit hitBuffer[64];
	physx::PxHitFlags pxFlags(flags);
	
	PxLuaRaycastCallback callback(l,oCallback,hitBuffer,64);
	pxScene->raycast(
		pxOrigin,
		pxDir,
		distance,
		callback
	);
}
#endif
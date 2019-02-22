#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"

extern DLLENGINE Engine *engine;

namespace Lua
{
	namespace PhysCollisionObj
	{
		static void IsValid(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void Remove(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void SetPos(lua_State *l,PhysCollisionObjectHandle &hPhys,Vector3 &pos);
		static void GetPos(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void SetRotation(lua_State *l,PhysCollisionObjectHandle &hPhys,Quat &rot);
		static void GetRotation(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void GetOrigin(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void SetOrigin(lua_State *l,PhysCollisionObjectHandle &hPhys,const Vector3 &origin);
		static void GetBounds(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void GetBoundingSphere(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void IsRigid(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void IsGhost(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void IsSoftBody(lua_State *l,PhysCollisionObjectHandle &hPhys);

		static void SetAngles(lua_State *l,PhysCollisionObjectHandle &hPhys,const EulerAngles &ang);
		static void GetAngles(lua_State *l,PhysCollisionObjectHandle &hPhys);

		static void SetSimulationEnabled(lua_State *l,PhysCollisionObjectHandle &hPhys,bool b);
		static void DisableSimulation(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void EnableSimulation(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void IsSimulationEnabled(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void GetShape(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void GetActivationState(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void SetActivationState(lua_State *l,PhysCollisionObjectHandle &hPhys,int32_t state);
		static void Activate(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void Activate(lua_State *l,PhysCollisionObjectHandle &hPhys,bool bForceActivation);
		static void Spawn(lua_State *l,PhysCollisionObjectHandle &hPhys);

		static void SetCollisionFilterGroup(lua_State *l,PhysCollisionObjectHandle &hPhys,uint32_t group);
		static void GetCollisionFilterGroup(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void SetCollisionFilterMask(lua_State *l,PhysCollisionObjectHandle &hPhys,uint32_t mask);
		static void GetCollisionFilterMask(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void GetWorldTransform(lua_State *l,PhysCollisionObjectHandle &hPhys);
		static void SetWorldTransform(lua_State *l,PhysCollisionObjectHandle &hPhys,const PhysTransform &t);
	};
	namespace PhysRigidBody
	{
		static void register_class(lua_State *l,luabind::class_<PhysCollisionObjectHandle> &classDefColObj,luabind::module_ &mod);
		static void SetLinearVelocity(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &vel);
		static void GetLinearVelocity(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void SetAngularVelocity(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &vel);
		static void GetAngularVelocity(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void SetMass(lua_State *l,PhysRigidBodyHandle &hPhys,float mass);
		static void GetMass(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void GetInertia(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void GetInvInertiaTensorWorld(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void ApplyForce(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &force);
		static void ApplyForce(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &force,Vector3 &relPos);
		static void ApplyImpulse(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &impulse);
		static void ApplyImpulse(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &impulse,Vector3 &relPos);
		static void ApplyTorque(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &torque);
		static void ApplyTorqueImpulse(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &torque);
		static void ClearForces(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void GetTotalForce(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void GetTotalTorque(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void GetBoneID(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void SetBoneID(lua_State *l,PhysRigidBodyHandle &hPhys,UInt32 boneId);

		static void SetDamping(lua_State *l,PhysRigidBodyHandle &hPhys,float linDamping,float angDamping);
		static void SetLinearDamping(lua_State *l,PhysRigidBodyHandle &hPhys,float linDamping);
		static void SetAngularDamping(lua_State *l,PhysRigidBodyHandle &hPhys,float angDamping);
		static void GetLinearDamping(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void GetAngularDamping(lua_State *l,PhysRigidBodyHandle &hPhys);

		static void SetLinearFactor(lua_State *l,PhysRigidBodyHandle &hPhys,const Vector3 &factor);
		static void GetLinearFactor(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void SetAngularFactor(lua_State *l,PhysRigidBodyHandle &hPhys,const Vector3 &factor);
		static void GetAngularFactor(lua_State *l,PhysRigidBodyHandle &hPhys);

		static void SetLinearSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys,float threshold);
		static void SetAngularSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys,float threshold);
		static void SetSleepingThresholds(lua_State *l,PhysRigidBodyHandle &hPhys,float linear,float angular);
		static void GetLinearSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void GetAngularSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys);
		static void GetSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys);
	};
	namespace PhysSoftBody
	{
		static void register_class(lua_State *l,luabind::class_<PhysCollisionObjectHandle> &classDefColObj,luabind::module_ &mod);
		static void AddAeroForceToNode(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node,const Vector3 &force);
		static void AddAeroForceToFace(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t face,const Vector3 &force);
		static void ApplyForce(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &force);
		static void ApplyForce(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node,const Vector3 &force);
		static void AddLinearVelocity(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &vel);
		static void AddLinearVelocity(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node,const Vector3 &vel);
		static void GetFriction(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetHitFraction(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetRollingFriction(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetAnisotropicFriction(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void SetFriction(lua_State *l,PhysSoftBodyHandle &hPhys,float friction);
		static void SetHitFraction(lua_State *l,PhysSoftBodyHandle &hPhys,float fraction);
		static void SetRollingFriction(lua_State *l,PhysSoftBodyHandle &hPhys,float friction);
		static void SetAnisotropicFriction(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &friction);
		static void GetMass(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node);
		static void GetMass(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetRestitution(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetRestLengthScale(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetWindVelocity(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void SetMass(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node,float mass);
		static void SetMass(lua_State *l,PhysSoftBodyHandle &hPhys,float mass);
		static void SetRestitution(lua_State *l,PhysSoftBodyHandle &hPhys,float rest);
		static void SetRestLengthScale(lua_State *l,PhysSoftBodyHandle &hPhys,float scale);
		static void SetWindVelocity(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &vel);
		static void SetLinearVelocity(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &vel);
		static void SetVolumeDensity(lua_State *l,PhysSoftBodyHandle &hPhys,float density);
		static void SetVolumeMass(lua_State *l,PhysSoftBodyHandle &hPhys,float mass);
		static void GetVolume(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void SetDensity(lua_State *l,PhysSoftBodyHandle &hPhys,float density);

		static void SetAnchorsHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetRigidContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetDynamicFrictionCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetDragCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetDampingCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetKineticContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetLiftCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetPoseMatchingCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetPressureCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetSoftContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetSoftVsKineticHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetSoftVsRigidImpulseSplitK(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetSoftVsRigidHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetSoftVsRigidImpulseSplitR(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetSoftVsSoftHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetSoftVsRigidImpulseSplitS(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetVolumeConversationCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val);
		static void SetVelocitiesCorrectionFactor(lua_State *l,PhysSoftBodyHandle &hPhys,float val);

		static void GetAnchorsHardness(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetRigidContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetDynamicFrictionCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetDragCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetDampingCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetKineticContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetLiftCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetPoseMatchingCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetPressureCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetSoftContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetSoftVsKineticHardness(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetSoftVsRigidImpulseSplitK(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetSoftVsRigidHardness(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetSoftVsRigidImpulseSplitR(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetSoftVsSoftHardness(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetSoftVsRigidImpulseSplitS(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetVolumeConversationCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys);
		static void GetVelocitiesCorrectionFactor(lua_State *l,PhysSoftBodyHandle &hPhys);

		static void SetMaterialAngularStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId,float val);
		static void SetMaterialLinearStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId,float val);
		static void SetMaterialVolumeStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId,float val);
		static void GetMaterialAngularStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId);
		static void GetMaterialLinearStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId);
		static void GetMaterialVolumeStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId);

		static void AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,const Vector3 &localPivot,bool bDisableCollision,float influence);
		static void AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,const Vector3 &localPivot,bool bDisableCollision);
		static void AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,const Vector3 &localPivot);
		static void AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,bool bDisableCollision,float influence);
		static void AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,bool bDisableCollision);
		static void AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody);
		static void GetNodeCount(lua_State *l,PhysSoftBodyHandle &hPhys);

		static void MeshVertexIndexToLocalVertexIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t meshVertexIndex);
		static void LocalVertexIndexToMeshVertexIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t localIndex);
		static void LocalVertexIndexToNodeIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t localVertexIndex);
		static void NodeIndexToLocalVertexIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeIndex);
	
		static void MeshVertexIndexToNodeIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t meshVertexIndex);
		static void NodeIndexToMeshVertexIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeIndex);
	};
};

void Lua::PhysCollisionObj::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<PhysCollisionObjectHandle>("CollisionObj");
	classDef.def("IsValid",&IsValid);
	classDef.def("Remove",&Remove);
	classDef.def("SetPos",&SetPos);
	classDef.def("GetPos",&GetPos);
	classDef.def("SetRotation",&SetRotation);
	classDef.def("GetRotation",&GetRotation);
	classDef.def("GetOrigin",&GetOrigin);
	classDef.def("SetOrigin",&SetOrigin);
	classDef.def("GetBounds",&GetBounds);
	classDef.def("GetBoundingSphere",&GetBoundingSphere);
	classDef.def("IsRigidBody",&IsRigid);
	classDef.def("IsGhostObject",&IsGhost);
	classDef.def("IsSoftBody",&IsSoftBody);
	classDef.def("SetSimulationEnabled",&SetSimulationEnabled);
	classDef.def("DisableSimulation",&DisableSimulation);
	classDef.def("EnableSimulation",&EnableSimulation);
	classDef.def("IsSimulationEnabled",&IsSimulationEnabled);
	classDef.def("GetShape",&GetShape);
	classDef.def("GetActivationState",&GetActivationState);
	classDef.def("SetActivationState",&SetActivationState);
	classDef.def("Activate",static_cast<void(*)(lua_State*,PhysCollisionObjectHandle&,bool)>(&Activate));
	classDef.def("Activate",static_cast<void(*)(lua_State*,PhysCollisionObjectHandle&)>(&Activate));
	classDef.def("Spawn",&Spawn);
	classDef.def("SetCollisionFilterGroup",&SetCollisionFilterGroup);
	classDef.def("GetCollisionFilterGroup",&GetCollisionFilterGroup);
	classDef.def("SetCollisionFilterMask",&SetCollisionFilterMask);
	classDef.def("GetCollisionFilterMask",&GetCollisionFilterMask);
	classDef.def("GetWorldTransform",&GetWorldTransform);
	classDef.def("SetWorldTransform",&SetWorldTransform);
	classDef.def("SetAngles",&SetAngles);
	classDef.def("GetAngles",&GetAngles);
	mod[classDef];

	PhysRigidBody::register_class(l,classDef,mod);
	PhysSoftBody::register_class(l,classDef,mod);
}

void Lua::PhysRigidBody::register_class(lua_State *l,luabind::class_<PhysCollisionObjectHandle> &classDefColObj,luabind::module_ &mod)
{
	auto classDef = luabind::class_<PhysRigidBodyHandle COMMA PhysCollisionObjectHandle>("RigidBody");
	classDef.def("SetLinearVelocity",&SetLinearVelocity);
	classDef.def("GetLinearVelocity",&GetLinearVelocity);
	classDef.def("SetAngularVelocity",&SetAngularVelocity);
	classDef.def("GetAngularVelocity",&GetAngularVelocity);
	classDef.def("SetMass",&SetMass);
	classDef.def("GetMass",&GetMass);
	classDef.def("GetInertia",&GetInertia);
	classDef.def("GetInvInertiaTensorWorld",&GetInvInertiaTensorWorld);
	classDef.def("ApplyForce",static_cast<void(*)(lua_State*,PhysRigidBodyHandle&,Vector3&)>(&ApplyForce));
	classDef.def("ApplyForce",static_cast<void(*)(lua_State*,PhysRigidBodyHandle&,Vector3&,Vector3&)>(&ApplyForce));
	classDef.def("ApplyImpulse",static_cast<void(*)(lua_State*,PhysRigidBodyHandle&,Vector3&)>(&ApplyImpulse));
	classDef.def("ApplyImpulse",static_cast<void(*)(lua_State*,PhysRigidBodyHandle&,Vector3&,Vector3&)>(&ApplyImpulse));
	classDef.def("ApplyTorque",&ApplyTorque);
	classDef.def("ApplyTorqueImpulse",&ApplyTorqueImpulse);
	classDef.def("ClearForces",&ClearForces);
	classDef.def("GetTotalForce",&GetTotalForce);
	classDef.def("GetTotalTorque",&GetTotalTorque);
	classDef.def("SetDamping",&SetDamping);
	classDef.def("SetLinearDamping",&SetLinearDamping);
	classDef.def("SetAngularDamping",&SetAngularDamping);
	classDef.def("GetLinearDamping",&GetLinearDamping);
	classDef.def("GetAngularDamping",&GetAngularDamping);
	classDef.def("GetBoneID",&GetBoneID);
	classDef.def("SetBoneID",&SetBoneID);
	classDef.def("SetLinearFactor",&SetLinearFactor);
	classDef.def("GetLinearFactor",&GetLinearFactor);
	classDef.def("SetAngularFactor",&SetAngularFactor);
	classDef.def("GetAngularFactor",&GetAngularFactor);
	classDef.def("SetLinearSleepingThreshold",&SetLinearSleepingThreshold);
	classDef.def("SetAngularSleepingThreshold",&SetAngularSleepingThreshold);
	classDef.def("SetSleepingThresholds",&SetSleepingThresholds);
	classDef.def("GetLinearSleepingThreshold",&GetLinearSleepingThreshold);
	classDef.def("GetAngularSleepingThreshold",&GetAngularSleepingThreshold);
	classDef.def("GetSleepingThreshold",&GetSleepingThreshold);
	classDef.def("SetKinematic",static_cast<void(*)(lua_State*,PhysRigidBodyHandle&,bool)>([](lua_State *l,PhysRigidBodyHandle &hPhys,bool bKinematic) {
		LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
		static_cast<::PhysRigidBody*>(hPhys.get())->SetKinematic(bKinematic);
	}));
	classDef.def("IsKinematic",static_cast<void(*)(lua_State*,PhysRigidBodyHandle&)>([](lua_State *l,PhysRigidBodyHandle &hPhys) {
		LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
		Lua::PushBool(l,static_cast<::PhysRigidBody*>(hPhys.get())->IsKinematic());
	}));
	mod[classDef];
}
void Lua::PhysSoftBody::register_class(lua_State *l,luabind::class_<PhysCollisionObjectHandle> &classDefColObj,luabind::module_ &mod)
{
	auto classDef = luabind::class_<PhysSoftBodyHandle COMMA PhysCollisionObjectHandle>("SoftBody");
	classDef.def("AddAeroForceToNode",&AddAeroForceToNode);
	classDef.def("AddAeroForceToFace",&AddAeroForceToFace);
	classDef.def("ApplyForce",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t,const Vector3&)>(&ApplyForce));
	classDef.def("ApplyForce",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,const Vector3&)>(&ApplyForce));
	classDef.def("AddLinearVelocity",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t,const Vector3&)>(&AddLinearVelocity));
	classDef.def("AddLinearVelocity",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,const Vector3&)>(&AddLinearVelocity));
	classDef.def("GetFriction",&GetFriction);
	classDef.def("GetHitFraction",&GetHitFraction);
	classDef.def("GetRollingFriction",&GetRollingFriction);
	classDef.def("GetAnisotropicFriction",&GetAnisotropicFriction);
	classDef.def("SetFriction",&SetFriction);
	classDef.def("SetHitFraction",&SetHitFraction);
	classDef.def("SetRollingFriction",&SetRollingFriction);
	classDef.def("SetAnisotropicFriction",&SetAnisotropicFriction);
	classDef.def("GetMass",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t)>(&GetMass));
	classDef.def("GetMass",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&)>(&GetMass));
	classDef.def("GetRestitution",&GetRestitution);
	classDef.def("GetRestLengthScale",&GetRestLengthScale);
	classDef.def("GetWindVelocity",&GetWindVelocity);
	classDef.def("SetMass",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t,float)>(&SetMass));
	classDef.def("SetMass",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,float)>(&SetMass));
	classDef.def("SetRestitution",&SetRestitution);
	classDef.def("SetRestLengthScale",&SetRestLengthScale);
	classDef.def("SetWindVelocity",&SetWindVelocity);
	classDef.def("SetLinearVelocity",&SetLinearVelocity);
	classDef.def("SetVolumeDensity",&SetVolumeDensity);
	classDef.def("SetVolumeMass",&SetVolumeMass);
	classDef.def("GetVolume",&GetVolume);
	classDef.def("SetDensity",&SetDensity);

	classDef.def("SetAnchorsHardness",&SetAnchorsHardness);
	classDef.def("SetRigidContactsHardness",&SetRigidContactsHardness);
	classDef.def("SetDynamicFrictionCoefficient",&SetDynamicFrictionCoefficient);
	classDef.def("SetDragCoefficient",&SetDragCoefficient);
	classDef.def("SetDampingCoefficient",&SetDampingCoefficient);
	classDef.def("SetKineticContactsHardness",&SetKineticContactsHardness);
	classDef.def("SetLiftCoefficient",&SetLiftCoefficient);
	classDef.def("SetPoseMatchingCoefficient",&SetPoseMatchingCoefficient);
	classDef.def("SetPressureCoefficient",&SetPressureCoefficient);
	classDef.def("SetSoftContactsHardness",&SetSoftContactsHardness);
	classDef.def("SetSoftVsKineticHardness",&SetSoftVsKineticHardness);
	classDef.def("SetSoftVsRigidImpulseSplitK",&SetSoftVsRigidImpulseSplitK);
	classDef.def("SetSoftVsRigidHardness",&SetSoftVsRigidHardness);
	classDef.def("SetSoftVsRigidImpulseSplitR",&SetSoftVsRigidImpulseSplitR);
	classDef.def("SetSoftVsSoftHardness",&SetSoftVsSoftHardness);
	classDef.def("SetSoftVsRigidImpulseSplitS",&SetSoftVsRigidImpulseSplitS);
	classDef.def("SetVolumeConversationCoefficient",&SetVolumeConversationCoefficient);
	classDef.def("SetVelocitiesCorrectionFactor",&SetVelocitiesCorrectionFactor);
	classDef.def("GetAnchorsHardness",&GetAnchorsHardness);
	classDef.def("GetRigidContactsHardness",&GetRigidContactsHardness);
	classDef.def("GetDynamicFrictionCoefficient",&GetDynamicFrictionCoefficient);
	classDef.def("GetDragCoefficient",&GetDragCoefficient);
	classDef.def("GetDampingCoefficient",&GetDampingCoefficient);
	classDef.def("GetKineticContactsHardness",&GetKineticContactsHardness);
	classDef.def("GetLiftCoefficient",&GetLiftCoefficient);
	classDef.def("GetPoseMatchingCoefficient",&GetPoseMatchingCoefficient);
	classDef.def("GetPressureCoefficient",&GetPressureCoefficient);
	classDef.def("GetSoftContactsHardness",&GetSoftContactsHardness);
	classDef.def("GetSoftVsKineticHardness",&GetSoftVsKineticHardness);
	classDef.def("GetSoftVsRigidImpulseSplitK",&GetSoftVsRigidImpulseSplitK);
	classDef.def("GetSoftVsRigidHardness",&GetSoftVsRigidHardness);
	classDef.def("GetSoftVsRigidImpulseSplitR",&GetSoftVsRigidImpulseSplitR);
	classDef.def("GetSoftVsSoftHardness",&GetSoftVsSoftHardness);
	classDef.def("GetSoftVsRigidImpulseSplitS",&GetSoftVsRigidImpulseSplitS);
	classDef.def("GetVolumeConversationCoefficient",&GetVolumeConversationCoefficient);
	classDef.def("GetVelocitiesCorrectionFactor",&GetVelocitiesCorrectionFactor);
	classDef.def("SetMaterialAngularStiffnessCoefficient",&SetMaterialAngularStiffnessCoefficient);
	classDef.def("SetMaterialLinearStiffnessCoefficient",&SetMaterialLinearStiffnessCoefficient);
	classDef.def("SetMaterialVolumeStiffnessCoefficient",&SetMaterialVolumeStiffnessCoefficient);
	classDef.def("GetMaterialAngularStiffnessCoefficient",&GetMaterialAngularStiffnessCoefficient);
	classDef.def("GetMaterialLinearStiffnessCoefficient",&GetMaterialLinearStiffnessCoefficient);
	classDef.def("GetMaterialVolumeStiffnessCoefficient",&GetMaterialVolumeStiffnessCoefficient);
	classDef.def("AppendAnchor",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t,PhysRigidBodyHandle&,const Vector3&,bool,float)>(&AppendAnchor));
	classDef.def("AppendAnchor",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t,PhysRigidBodyHandle&,const Vector3&,bool)>(&AppendAnchor));
	classDef.def("AppendAnchor",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t,PhysRigidBodyHandle&,const Vector3&)>(&AppendAnchor));
	classDef.def("AppendAnchor",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t,PhysRigidBodyHandle&,bool,float)>(&AppendAnchor));
	classDef.def("AppendAnchor",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t,PhysRigidBodyHandle&,bool)>(&AppendAnchor));
	classDef.def("AppendAnchor",static_cast<void(*)(lua_State*,PhysSoftBodyHandle&,uint32_t,PhysRigidBodyHandle&)>(&AppendAnchor));
	classDef.def("GetNodeCount",&GetNodeCount);

	classDef.def("MeshVertexIndexToLocalVertexIndex",&MeshVertexIndexToLocalVertexIndex);
	classDef.def("LocalVertexIndexToMeshVertexIndex",&LocalVertexIndexToMeshVertexIndex);
	classDef.def("LocalVertexIndexToNodeIndex",&LocalVertexIndexToNodeIndex);
	classDef.def("NodeIndexToLocalVertexIndex",&NodeIndexToLocalVertexIndex);
	classDef.def("MeshVertexIndexToNodeIndex",&MeshVertexIndexToNodeIndex);
	classDef.def("NodeIndexToMeshVertexIndex",&NodeIndexToMeshVertexIndex);
	mod[classDef];
}

/////////////////////////////////////////////

void Lua::PhysCollisionObj::IsValid(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	Lua::PushBool(l,hPhys.IsValid());
}
void Lua::PhysCollisionObj::Remove(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	delete hPhys.get();
}
void Lua::PhysCollisionObj::SetPos(lua_State *l,PhysCollisionObjectHandle &hPhys,Vector3 &pos)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->SetPos(pos);
}
void Lua::PhysCollisionObj::GetPos(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,hPhys->GetPos());
}
void Lua::PhysCollisionObj::GetBoundingSphere(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	auto *colObj = hPhys->GetCollisionObject();
	if(colObj == nullptr)
		return;
	auto *shape = colObj->getCollisionShape();
	if(shape == nullptr)
		return;
	auto &t = colObj->getWorldTransform();
	btVector3 center;
	btScalar radius;
	shape->getBoundingSphere(center,radius);
	center += t.getOrigin();
	center /= PhysEnv::WORLD_SCALE;
	radius /= PhysEnv::WORLD_SCALE;
	Lua::Push<Vector3>(l,Vector3{center.x(),center.y(),center.z()});
	Lua::PushNumber(l,radius);
}
void Lua::PhysCollisionObj::IsRigid(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushBool(l,hPhys->IsRigid());
}
void Lua::PhysCollisionObj::IsGhost(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushBool(l,hPhys->IsGhost());
}
void Lua::PhysCollisionObj::IsSoftBody(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushBool(l,hPhys->IsSoftBody());
}
void Lua::PhysCollisionObj::SetAngles(lua_State *l,PhysCollisionObjectHandle &hPhys,const EulerAngles &ang)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->SetRotation(uquat::create(ang));
}
void Lua::PhysCollisionObj::GetAngles(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<EulerAngles>(l,EulerAngles(hPhys->GetRotation()));
}
void Lua::PhysCollisionObj::SetSimulationEnabled(lua_State *l,PhysCollisionObjectHandle &hPhys,bool b)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->SetSimulationEnabled(b);
}
void Lua::PhysCollisionObj::DisableSimulation(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->DisableSimulation();
}
void Lua::PhysCollisionObj::EnableSimulation(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->EnableSimulation();
}
void Lua::PhysCollisionObj::IsSimulationEnabled(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushBool(l,hPhys->IsSimulationEnabled());
}
void Lua::PhysCollisionObj::GetShape(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	auto shape = hPhys->GetCollisionShape();
	if(shape == nullptr)
		return;
	auto o = shape->GetLuaObject(l);
	o.push(l);
}
void Lua::PhysCollisionObj::GetActivationState(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushInt(l,hPhys->GetActivationState());
}
void Lua::PhysCollisionObj::SetActivationState(lua_State *l,PhysCollisionObjectHandle &hPhys,int32_t state)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->SetActivationState(state);
}
void Lua::PhysCollisionObj::Activate(lua_State *l,PhysCollisionObjectHandle &hPhys,bool bForceActivation)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->Activate(bForceActivation);
}
void Lua::PhysCollisionObj::Activate(lua_State *l,PhysCollisionObjectHandle &hPhys) {Activate(l,hPhys,false);}
void Lua::PhysCollisionObj::Spawn(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->Spawn();
}
void Lua::PhysCollisionObj::SetCollisionFilterGroup(lua_State *l,PhysCollisionObjectHandle &hPhys,uint32_t group)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->SetCollisionFilterGroup(static_cast<CollisionMask>(group));
}
void Lua::PhysCollisionObj::GetCollisionFilterGroup(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushInt(l,hPhys->GetCollisionFilterGroup());
}
void Lua::PhysCollisionObj::SetCollisionFilterMask(lua_State *l,PhysCollisionObjectHandle &hPhys,uint32_t mask)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->SetCollisionFilterMask(static_cast<CollisionMask>(mask));
}
void Lua::PhysCollisionObj::GetCollisionFilterMask(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushInt(l,hPhys->GetCollisionFilterMask());
}
void Lua::PhysCollisionObj::GetWorldTransform(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<PhysTransform>(l,hPhys->GetWorldTransform());
}
void Lua::PhysCollisionObj::SetWorldTransform(lua_State *l,PhysCollisionObjectHandle &hPhys,const PhysTransform &t)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->SetWorldTransform(t);
}
void Lua::PhysCollisionObj::GetBounds(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Vector3 min,max;
	hPhys->GetAABB(min,max);
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
}
void Lua::PhysCollisionObj::SetRotation(lua_State *l,PhysCollisionObjectHandle &hPhys,Quat &rot)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->SetRotation(rot);
}
void Lua::PhysCollisionObj::GetRotation(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Quat>(l,hPhys->GetRotation());
}
void Lua::PhysCollisionObj::GetOrigin(lua_State *l,PhysCollisionObjectHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,hPhys->GetOrigin());
}
void Lua::PhysCollisionObj::SetOrigin(lua_State *l,PhysCollisionObjectHandle &hPhys,const Vector3 &origin)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	hPhys->SetOrigin(origin);
}

/////////////////////////////////////////////

void Lua::PhysRigidBody::SetLinearVelocity(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &vel)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetLinearVelocity(vel);
}
void Lua::PhysRigidBody::GetLinearVelocity(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetLinearVelocity());
}
void Lua::PhysRigidBody::SetAngularVelocity(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &vel)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetAngularVelocity(vel);
}
void Lua::PhysRigidBody::GetAngularVelocity(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetAngularVelocity());
}
void Lua::PhysRigidBody::SetMass(lua_State *l,PhysRigidBodyHandle &hPhys,float mass)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetMass(mass);
}
void Lua::PhysRigidBody::GetMass(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetMass());
}
void Lua::PhysRigidBody::GetInertia(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetInertia());
}
void Lua::PhysRigidBody::GetInvInertiaTensorWorld(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Mat3>(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetInvInertiaTensorWorld());
}
void Lua::PhysRigidBody::ApplyForce(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &force)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->ApplyForce(force);
}
void Lua::PhysRigidBody::ApplyForce(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &force,Vector3 &relPos)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->ApplyForce(force,relPos);
}
void Lua::PhysRigidBody::ApplyImpulse(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &impulse)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->ApplyImpulse(impulse);
}
void Lua::PhysRigidBody::ApplyImpulse(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &impulse,Vector3 &relPos)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->ApplyImpulse(impulse,relPos);
}
void Lua::PhysRigidBody::ApplyTorque(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &torque)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->ApplyTorque(torque);
}
void Lua::PhysRigidBody::ApplyTorqueImpulse(lua_State *l,PhysRigidBodyHandle &hPhys,Vector3 &torque)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->ApplyTorqueImpulse(torque);
}
void Lua::PhysRigidBody::ClearForces(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->ClearForces();
}
void Lua::PhysRigidBody::GetTotalForce(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetTotalForce());
}
void Lua::PhysRigidBody::GetTotalTorque(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetTotalTorque());
}
void Lua::PhysRigidBody::SetDamping(lua_State *l,PhysRigidBodyHandle &hPhys,float linDamping,float angDamping)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetDamping(linDamping,angDamping);
}
void Lua::PhysRigidBody::SetLinearDamping(lua_State *l,PhysRigidBodyHandle &hPhys,float linDamping)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetLinearDamping(linDamping);
}
void Lua::PhysRigidBody::SetAngularDamping(lua_State *l,PhysRigidBodyHandle &hPhys,float angDamping)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetAngularDamping(angDamping);
}
void Lua::PhysRigidBody::GetLinearDamping(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetLinearDamping());
}
void Lua::PhysRigidBody::GetAngularDamping(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetAngularDamping());
}
void Lua::PhysRigidBody::GetBoneID(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetBoneID());
}
void Lua::PhysRigidBody::SetBoneID(lua_State *l,PhysRigidBodyHandle &hPhys,UInt32 boneId)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetBoneID(boneId);
}
void Lua::PhysRigidBody::SetLinearFactor(lua_State *l,PhysRigidBodyHandle &hPhys,const Vector3 &factor)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetLinearFactor(factor);
}
void Lua::PhysRigidBody::GetLinearFactor(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetLinearFactor());
}
void Lua::PhysRigidBody::SetAngularFactor(lua_State *l,PhysRigidBodyHandle &hPhys,const Vector3 &factor)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetAngularFactor(factor);
}
void Lua::PhysRigidBody::GetAngularFactor(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetAngularFactor());
}
void Lua::PhysRigidBody::SetLinearSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys,float threshold)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetLinearSleepingThreshold(threshold);
}
void Lua::PhysRigidBody::SetAngularSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys,float threshold)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetAngularSleepingThreshold(threshold);
}
void Lua::PhysRigidBody::SetSleepingThresholds(lua_State *l,PhysRigidBodyHandle &hPhys,float linear,float angular)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysRigidBody*>(hPhys.get())->SetSleepingThresholds(linear,angular);
}
void Lua::PhysRigidBody::GetLinearSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetLinearSleepingThreshold());
}
void Lua::PhysRigidBody::GetAngularSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysRigidBody*>(hPhys.get())->GetAngularSleepingThreshold());
}
void Lua::PhysRigidBody::GetSleepingThreshold(lua_State *l,PhysRigidBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	auto thresholds = static_cast<::PhysRigidBody*>(hPhys.get())->GetSleepingThreshold();
	Lua::PushNumber(l,thresholds.first);
	Lua::PushNumber(l,thresholds.second);
}

/////////////////////////////////////////////

void Lua::PhysSoftBody::AddAeroForceToNode(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node,const Vector3 &force)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->AddAeroForceToNode(node,force);
}
void Lua::PhysSoftBody::AddAeroForceToFace(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t face,const Vector3 &force)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->AddAeroForceToFace(face,force);
}
void Lua::PhysSoftBody::ApplyForce(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &force)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->AddForce(force);
}
void Lua::PhysSoftBody::ApplyForce(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node,const Vector3 &force)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->AddForce(node,force);
}
void Lua::PhysSoftBody::AddLinearVelocity(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &vel)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->AddLinearVelocity(vel);
}
void Lua::PhysSoftBody::AddLinearVelocity(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node,const Vector3 &vel)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->AddLinearVelocity(node,vel);
}
void Lua::PhysSoftBody::GetFriction(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetFriction());
}
void Lua::PhysSoftBody::GetHitFraction(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetHitFraction());
}
void Lua::PhysSoftBody::GetRollingFriction(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetRollingFriction());
}
void Lua::PhysSoftBody::GetAnisotropicFriction(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetAnisotropicFriction());
}
void Lua::PhysSoftBody::SetFriction(lua_State *l,PhysSoftBodyHandle &hPhys,float friction)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetFriction(friction);
}
void Lua::PhysSoftBody::SetHitFraction(lua_State *l,PhysSoftBodyHandle &hPhys,float fraction)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetHitFraction(fraction);
}
void Lua::PhysSoftBody::SetRollingFriction(lua_State *l,PhysSoftBodyHandle &hPhys,float friction)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetRollingFriction(friction);
}
void Lua::PhysSoftBody::SetAnisotropicFriction(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &friction)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetAnisotropicFriction(friction);
}
void Lua::PhysSoftBody::GetMass(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetMass(node));
}
void Lua::PhysSoftBody::GetMass(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetMass());
}
void Lua::PhysSoftBody::GetRestitution(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetRestitution());
}
void Lua::PhysSoftBody::GetRestLengthScale(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetRestLengthScale());
}
void Lua::PhysSoftBody::GetWindVelocity(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::Push<Vector3>(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetWindVelocity());
}
void Lua::PhysSoftBody::SetMass(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t node,float mass)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetMass(node,mass);
}
void Lua::PhysSoftBody::SetMass(lua_State *l,PhysSoftBodyHandle &hPhys,float mass)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetMass(mass);
}
void Lua::PhysSoftBody::SetRestitution(lua_State *l,PhysSoftBodyHandle &hPhys,float rest)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetRestitution(rest);
}
void Lua::PhysSoftBody::SetRestLengthScale(lua_State *l,PhysSoftBodyHandle &hPhys,float scale)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetRestLengthScale(scale);
}
void Lua::PhysSoftBody::SetWindVelocity(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &vel)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetWindVelocity(vel);
}
void Lua::PhysSoftBody::SetLinearVelocity(lua_State *l,PhysSoftBodyHandle &hPhys,const Vector3 &vel)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetLinearVelocity(vel);
}
void Lua::PhysSoftBody::SetVolumeDensity(lua_State *l,PhysSoftBodyHandle &hPhys,float density)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetVolumeDensity(density);
}
void Lua::PhysSoftBody::SetVolumeMass(lua_State *l,PhysSoftBodyHandle &hPhys,float mass)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetVolumeMass(mass);
}
void Lua::PhysSoftBody::GetVolume(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetVolume());
}
void Lua::PhysSoftBody::SetDensity(lua_State *l,PhysSoftBodyHandle &hPhys,float density)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetDensity(density);
}

void Lua::PhysSoftBody::SetAnchorsHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetAnchorsHardness(val);
}
void Lua::PhysSoftBody::SetRigidContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetRigidContactsHardness(val);
}
void Lua::PhysSoftBody::SetDynamicFrictionCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetDynamicFrictionCoefficient(val);
}
void Lua::PhysSoftBody::SetDragCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetDragCoefficient(val);
}
void Lua::PhysSoftBody::SetDampingCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetDampingCoefficient(val);
}
void Lua::PhysSoftBody::SetKineticContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetKineticContactsHardness(val);
}
void Lua::PhysSoftBody::SetLiftCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetLiftCoefficient(val);
}
void Lua::PhysSoftBody::SetPoseMatchingCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetPoseMatchingCoefficient(val);
}
void Lua::PhysSoftBody::SetPressureCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetPressureCoefficient(val);
}
void Lua::PhysSoftBody::SetSoftContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetSoftContactsHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsKineticHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetSoftVsKineticHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidImpulseSplitK(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetSoftVsRigidImpulseSplitK(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetSoftVsRigidHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidImpulseSplitR(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetSoftVsRigidImpulseSplitR(val);
}
void Lua::PhysSoftBody::SetSoftVsSoftHardness(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetSoftVsSoftHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidImpulseSplitS(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetSoftVsRigidImpulseSplitS(val);
}
void Lua::PhysSoftBody::SetVolumeConversationCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetVolumeConversationCoefficient(val);
}
void Lua::PhysSoftBody::SetVelocitiesCorrectionFactor(lua_State *l,PhysSoftBodyHandle &hPhys,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetVelocitiesCorrectionFactor(val);
}

void Lua::PhysSoftBody::GetAnchorsHardness(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetAnchorsHardness());
}
void Lua::PhysSoftBody::GetRigidContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetRigidContactsHardness());
}
void Lua::PhysSoftBody::GetDynamicFrictionCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetDynamicFrictionCoefficient());
}
void Lua::PhysSoftBody::GetDragCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetDragCoefficient());
}
void Lua::PhysSoftBody::GetDampingCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetDampingCoefficient());
}
void Lua::PhysSoftBody::GetKineticContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetKineticContactsHardness());
}
void Lua::PhysSoftBody::GetLiftCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetLiftCoefficient());
}
void Lua::PhysSoftBody::GetPoseMatchingCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetPoseMatchingCoefficient());
}
void Lua::PhysSoftBody::GetPressureCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetPressureCoefficient());
}
void Lua::PhysSoftBody::GetSoftContactsHardness(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetSoftContactsHardness());
}
void Lua::PhysSoftBody::GetSoftVsKineticHardness(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetSoftVsKineticHardness());
}
void Lua::PhysSoftBody::GetSoftVsRigidImpulseSplitK(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetSoftVsRigidImpulseSplitK());
}
void Lua::PhysSoftBody::GetSoftVsRigidHardness(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetSoftVsRigidHardness());
}
void Lua::PhysSoftBody::GetSoftVsRigidImpulseSplitR(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetSoftVsRigidImpulseSplitR());
}
void Lua::PhysSoftBody::GetSoftVsSoftHardness(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetSoftVsSoftHardness());
}
void Lua::PhysSoftBody::GetSoftVsRigidImpulseSplitS(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetSoftVsRigidImpulseSplitS());
}
void Lua::PhysSoftBody::GetVolumeConversationCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetVolumeConversationCoefficient());
}
void Lua::PhysSoftBody::GetVelocitiesCorrectionFactor(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetVelocitiesCorrectionFactor());
}
void Lua::PhysSoftBody::SetMaterialAngularStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetMaterialAngularStiffnessCoefficient(matId,val);
}
void Lua::PhysSoftBody::SetMaterialLinearStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	static_cast<::PhysSoftBody*>(hPhys.get())->SetMaterialLinearStiffnessCoefficient(matId,val);
}
void Lua::PhysSoftBody::SetMaterialVolumeStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId,float val)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);;
	static_cast<::PhysSoftBody*>(hPhys.get())->SetMaterialVolumeStiffnessCoefficient(matId,val);
}
void Lua::PhysSoftBody::GetMaterialAngularStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetMaterialAngularStiffnessCoefficient(matId));
}
void Lua::PhysSoftBody::GetMaterialLinearStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetMaterialLinearStiffnessCoefficient(matId));
}
void Lua::PhysSoftBody::GetMaterialVolumeStiffnessCoefficient(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t matId)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushNumber(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetMaterialVolumeStiffnessCoefficient(matId));
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,const Vector3 &localPivot,bool bDisableCollision,float influence)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hRigidBody);
	static_cast<::PhysSoftBody*>(hPhys.get())->AppendAnchor(nodeId,*static_cast<::PhysRigidBody*>(hRigidBody.get()),localPivot,bDisableCollision,influence);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,const Vector3 &localPivot,bool bDisableCollision)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hRigidBody);
	static_cast<::PhysSoftBody*>(hPhys.get())->AppendAnchor(nodeId,*static_cast<::PhysRigidBody*>(hRigidBody.get()),localPivot,bDisableCollision);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,const Vector3 &localPivot)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hRigidBody);
	static_cast<::PhysSoftBody*>(hPhys.get())->AppendAnchor(nodeId,*static_cast<::PhysRigidBody*>(hRigidBody.get()),localPivot);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,bool bDisableCollision,float influence)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hRigidBody);
	static_cast<::PhysSoftBody*>(hPhys.get())->AppendAnchor(nodeId,*static_cast<::PhysRigidBody*>(hRigidBody.get()),bDisableCollision,influence);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody,bool bDisableCollision)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hRigidBody);
	static_cast<::PhysSoftBody*>(hPhys.get())->AppendAnchor(nodeId,*static_cast<::PhysRigidBody*>(hRigidBody.get()),bDisableCollision);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeId,PhysRigidBodyHandle &hRigidBody)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hRigidBody);
	static_cast<::PhysSoftBody*>(hPhys.get())->AppendAnchor(nodeId,*static_cast<::PhysRigidBody*>(hRigidBody.get()));
}
void Lua::PhysSoftBody::GetNodeCount(lua_State *l,PhysSoftBodyHandle &hPhys)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	Lua::PushInt(l,static_cast<::PhysSoftBody*>(hPhys.get())->GetNodeCount());
}
void Lua::PhysSoftBody::MeshVertexIndexToLocalVertexIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t meshVertexIndex)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	uint16_t localIndex = 0u;
	if(static_cast<::PhysSoftBody*>(hPhys.get())->MeshVertexIndexToLocalVertexIndex(meshVertexIndex,localIndex) == false)
		return;
	Lua::PushInt(l,localIndex);
}
void Lua::PhysSoftBody::LocalVertexIndexToMeshVertexIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t localIndex)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	uint16_t meshVertexIndex = 0u;
	if(static_cast<::PhysSoftBody*>(hPhys.get())->LocalVertexIndexToMeshVertexIndex(localIndex,meshVertexIndex) == false)
		return;
	Lua::PushInt(l,meshVertexIndex);
}
void Lua::PhysSoftBody::LocalVertexIndexToNodeIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t localVertexIndex)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	uint16_t nodeIndex = 0u;
	if(static_cast<::PhysSoftBody*>(hPhys.get())->LocalVertexIndexToNodeIndex(localVertexIndex,nodeIndex) == false)
		return;
	Lua::PushInt(l,nodeIndex);
}
void Lua::PhysSoftBody::NodeIndexToLocalVertexIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeIndex)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	uint16_t localIndex = 0u;
	if(static_cast<::PhysSoftBody*>(hPhys.get())->NodeIndexToLocalVertexIndex(nodeIndex,localIndex) == false)
		return;
	Lua::PushInt(l,localIndex);
}
	
void Lua::PhysSoftBody::MeshVertexIndexToNodeIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t meshVertexIndex)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	uint16_t nodeIndex = 0u;
	if(static_cast<::PhysSoftBody*>(hPhys.get())->MeshVertexIndexToNodeIndex(meshVertexIndex,nodeIndex) == false)
		return;
	Lua::PushInt(l,nodeIndex);
}
void Lua::PhysSoftBody::NodeIndexToMeshVertexIndex(lua_State *l,PhysSoftBodyHandle &hPhys,uint32_t nodeIndex)
{
	LUA_CHECK_PHYS_COLLISION_OBJ(l,hPhys);
	uint16_t meshIndex = 0u;
	if(static_cast<::PhysSoftBody*>(hPhys.get())->NodeIndexToMeshVertexIndex(nodeIndex,meshIndex) == false)
		return;
	Lua::PushInt(l,meshIndex);
}

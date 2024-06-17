/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/vehicle.hpp"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/lua/converters/game_type_converters_t.hpp"

#include "pragma/lua/ostream_operator_alias.hpp"

extern DLLNETWORK Engine *engine;

namespace Lua {
	namespace PhysCollisionObj {
		static void IsValid(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void Remove(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetPos(lua_State *l, pragma::physics::ICollisionObject *hPhys, Vector3 &pos);
		static void GetPos(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetRotation(lua_State *l, pragma::physics::ICollisionObject *hPhys, Quat &rot);
		static void GetRotation(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetOrigin(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetOrigin(lua_State *l, pragma::physics::ICollisionObject *hPhys, const Vector3 &origin);
		static void GetBounds(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetBoundingSphere(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void IsRigid(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void IsGhost(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void IsSoftBody(lua_State *l, pragma::physics::ICollisionObject *hPhys);

		static void SetAngles(lua_State *l, pragma::physics::ICollisionObject *hPhys, const EulerAngles &ang);
		static void GetAngles(lua_State *l, pragma::physics::ICollisionObject *hPhys);

		static void SetSimulationEnabled(lua_State *l, pragma::physics::ICollisionObject *hPhys, bool b);
		static void DisableSimulation(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void EnableSimulation(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void IsSimulationEnabled(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetShape(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetActivationState(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetActivationState(lua_State *l, pragma::physics::ICollisionObject *hPhys, int32_t state);
		static void Activate(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void Activate(lua_State *l, pragma::physics::ICollisionObject *hPhys, bool bForceActivation);
		static void Spawn(lua_State *l, pragma::physics::ICollisionObject *hPhys);

		static void SetCollisionFilterGroup(lua_State *l, pragma::physics::ICollisionObject *hPhys, uint32_t group);
		static void GetCollisionFilterGroup(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetCollisionFilterMask(lua_State *l, pragma::physics::ICollisionObject *hPhys, uint32_t mask);
		static void GetCollisionFilterMask(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetWorldTransform(lua_State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetWorldTransform(lua_State *l, pragma::physics::ICollisionObject *hPhys, const umath::Transform &t);
	};
	namespace PhysRigidBody {
		static void register_class(lua_State *l, luabind::module_ &mod);
		static void IsValid(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void SetLinearVelocity(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &vel);
		static void GetLinearVelocity(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void SetAngularVelocity(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &vel);
		static void GetAngularVelocity(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void SetMass(lua_State *l, pragma::physics::IRigidBody *hPhys, float mass);
		static void GetMass(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void GetInertia(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void GetInvInertiaTensorWorld(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void ApplyForce(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &force);
		static void ApplyForce(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &force, Vector3 &relPos);
		static void ApplyImpulse(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &impulse);
		static void ApplyImpulse(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &impulse, Vector3 &relPos);
		static void ApplyTorque(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &torque);
		static void ApplyTorqueImpulse(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &torque);
		static void ClearForces(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void GetTotalForce(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void GetTotalTorque(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void GetBoneID(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void SetBoneID(lua_State *l, pragma::physics::IRigidBody *hPhys, UInt32 boneId);

		static void SetDamping(lua_State *l, pragma::physics::IRigidBody *hPhys, float linDamping, float angDamping);
		static void SetLinearDamping(lua_State *l, pragma::physics::IRigidBody *hPhys, float linDamping);
		static void SetAngularDamping(lua_State *l, pragma::physics::IRigidBody *hPhys, float angDamping);
		static void GetLinearDamping(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void GetAngularDamping(lua_State *l, pragma::physics::IRigidBody *hPhys);

		static void SetLinearFactor(lua_State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &factor);
		static void GetLinearFactor(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void SetAngularFactor(lua_State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &factor);
		static void GetAngularFactor(lua_State *l, pragma::physics::IRigidBody *hPhys);

		static void SetLinearSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys, float threshold);
		static void SetAngularSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys, float threshold);
		static void SetSleepingThresholds(lua_State *l, pragma::physics::IRigidBody *hPhys, float linear, float angular);
		static void GetLinearSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void GetAngularSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys);
		static void GetSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys);
	};
	namespace PhysSoftBody {
		static void register_class(lua_State *l, luabind::module_ &mod);
		static void IsValid(lua_State *l, pragma::physics::ISoftBody *hPhys);
		static void AddAeroForceToNode(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &force);
		static void AddAeroForceToFace(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t face, const Vector3 &force);
		static void ApplyForce(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &force);
		static void ApplyForce(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &force);
		static void AddLinearVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel);
		static void AddLinearVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &vel);
		static void GetFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetHitFraction(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetRollingFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetAnisotropicFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void SetFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys, float friction);
		static void SetHitFraction(lua_State *l, pragma::physics::ISoftBody *&hPhys, float fraction);
		static void SetRollingFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys, float friction);
		static void SetAnisotropicFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &friction);
		static void GetMass(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node);
		static void GetMass(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetRestitution(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetRestLengthScale(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetWindVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void SetMass(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, float mass);
		static void SetMass(lua_State *l, pragma::physics::ISoftBody *&hPhys, float mass);
		static void SetRestitution(lua_State *l, pragma::physics::ISoftBody *&hPhys, float rest);
		static void SetRestLengthScale(lua_State *l, pragma::physics::ISoftBody *&hPhys, float scale);
		static void SetWindVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel);
		static void SetLinearVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel);
		static void SetVolumeDensity(lua_State *l, pragma::physics::ISoftBody *&hPhys, float density);
		static void SetVolumeMass(lua_State *l, pragma::physics::ISoftBody *&hPhys, float mass);
		static void GetVolume(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void SetDensity(lua_State *l, pragma::physics::ISoftBody *&hPhys, float density);

		static void SetAnchorsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetRigidContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetDynamicFrictionCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetDragCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetDampingCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetKineticContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetLiftCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetPoseMatchingCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetPressureCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsKineticHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsRigidImpulseSplitK(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsRigidHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsRigidImpulseSplitR(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsSoftHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsRigidImpulseSplitS(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetVolumeConversationCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetVelocitiesCorrectionFactor(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val);

		static void GetAnchorsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetRigidContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetDynamicFrictionCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetDragCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetDampingCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetKineticContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetLiftCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetPoseMatchingCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetPressureCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsKineticHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsRigidImpulseSplitK(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsRigidHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsRigidImpulseSplitR(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsSoftHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsRigidImpulseSplitS(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetVolumeConversationCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetVelocitiesCorrectionFactor(lua_State *l, pragma::physics::ISoftBody *&hPhys);

		static void SetMaterialAngularStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val);
		static void SetMaterialLinearStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val);
		static void SetMaterialVolumeStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val);
		static void GetMaterialAngularStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId);
		static void GetMaterialLinearStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId);
		static void GetMaterialVolumeStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId);

		static void AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot, bool bDisableCollision, float influence);
		static void AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot, bool bDisableCollision);
		static void AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot);
		static void AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, bool bDisableCollision, float influence);
		static void AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, bool bDisableCollision);
		static void AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody);
		static void GetNodeCount(lua_State *l, pragma::physics::ISoftBody *hPhys);

		static void MeshVertexIndexToLocalVertexIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t meshVertexIndex);
		static void LocalVertexIndexToMeshVertexIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t localIndex);
		static void LocalVertexIndexToNodeIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t localVertexIndex);
		static void NodeIndexToLocalVertexIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t nodeIndex);

		static void MeshVertexIndexToNodeIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t meshVertexIndex);
		static void NodeIndexToMeshVertexIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t nodeIndex);
	};
};

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::physics, ICollisionObject);
#endif

void Lua::PhysCollisionObj::register_class(lua_State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::physics::ICollisionObject, pragma::physics::IBase>("CollisionObj");
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("IsValid", &IsValid);
	classDef.def("Remove", &Remove);
	classDef.def("SetPos", &SetPos);
	classDef.def("GetPos", &GetPos);
	classDef.def("SetRotation", &SetRotation);
	classDef.def("GetRotation", &GetRotation);
	classDef.def("GetOrigin", &GetOrigin);
	classDef.def("SetOrigin", &SetOrigin);
	classDef.def("GetBounds", &GetBounds);
	classDef.def("GetBoundingSphere", &GetBoundingSphere);
	classDef.def("IsRigidBody", &IsRigid);
	classDef.def("IsGhostObject", &IsGhost);
	classDef.def("IsSoftBody", &IsSoftBody);
	classDef.def("SetSimulationEnabled", &SetSimulationEnabled);
	classDef.def("DisableSimulation", &DisableSimulation);
	classDef.def("EnableSimulation", &EnableSimulation);
	classDef.def("IsSimulationEnabled", &IsSimulationEnabled);
	classDef.def("GetShape", &GetShape);
	classDef.def("GetActivationState", &GetActivationState);
	classDef.def("SetActivationState", &SetActivationState);
	classDef.def("WakeUp", static_cast<void (*)(lua_State *, pragma::physics::ICollisionObject *, bool)>(&Activate));
	classDef.def("WakeUp", static_cast<void (*)(lua_State *, pragma::physics::ICollisionObject *)>(&Activate));
	classDef.def("Spawn", &Spawn);
	classDef.def("SetCollisionFilterGroup", &SetCollisionFilterGroup);
	classDef.def("GetCollisionFilterGroup", &GetCollisionFilterGroup);
	classDef.def("SetCollisionFilterMask", &SetCollisionFilterMask);
	classDef.def("GetCollisionFilterMask", &GetCollisionFilterMask);
	classDef.def("GetWorldTransform", &GetWorldTransform);
	classDef.def("SetWorldTransform", &SetWorldTransform);
	classDef.def("SetAngles", &SetAngles);
	classDef.def("GetAngles", &GetAngles);

	classDef.def("PutToSleep", static_cast<void (*)(lua_State *, pragma::physics::ICollisionObject *)>([](lua_State *l, pragma::physics::ICollisionObject *hPhys) {
		if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
			return;
		hPhys->PutToSleep();
	}));
	classDef.def("IsAsleep", static_cast<void (*)(lua_State *, pragma::physics::ICollisionObject *)>([](lua_State *l, pragma::physics::ICollisionObject *hPhys) {
		if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
			return;
		Lua::PushBool(l, hPhys->IsAsleep());
	}));
	classDef.def("SetAlwaysAwake", static_cast<void (*)(lua_State *, pragma::physics::ICollisionObject *, bool)>([](lua_State *l, pragma::physics::ICollisionObject *hPhys, bool alwaysAwake) {
		if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
			return;
		hPhys->SetAlwaysAwake(alwaysAwake);
	}));
	classDef.def("IsAlwaysAwake", static_cast<bool (*)(lua_State *, pragma::physics::ICollisionObject *)>([](lua_State *l, pragma::physics::ICollisionObject *hPhys) -> bool {
		if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
			return false;
		return hPhys->IsAlwaysAwake();
	}));

	mod[classDef];

	PhysRigidBody::register_class(l, mod);
	PhysSoftBody::register_class(l, mod);

	auto classDefGhost = luabind::class_<pragma::physics::IGhostObject, pragma::physics::IBase>("GhostObj");
	classDefGhost.def("IsValid", static_cast<void (*)(lua_State *, pragma::physics::IGhostObject *)>([](lua_State *l, pragma::physics::IGhostObject *hPhys) { Lua::PushBool(l, hPhys != nullptr); }));
	mod[classDefGhost];
}

void Lua::PhysRigidBody::register_class(lua_State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::physics::IRigidBody, luabind::bases<pragma::physics::ICollisionObject, pragma::physics::IBase>>("RigidBody");
	classDef.def("IsValid", &IsValid);
	classDef.def("SetLinearVelocity", &SetLinearVelocity);
	classDef.def("GetLinearVelocity", &GetLinearVelocity);
	classDef.def("SetAngularVelocity", &SetAngularVelocity);
	classDef.def("GetAngularVelocity", &GetAngularVelocity);
	classDef.def("SetMass", &SetMass);
	classDef.def("GetMass", &GetMass);
	classDef.def("GetInertia", &GetInertia);
	classDef.def("GetInvInertiaTensorWorld", &GetInvInertiaTensorWorld);
	classDef.def("ApplyForce", static_cast<void (*)(lua_State *, pragma::physics::IRigidBody *, Vector3 &)>(&ApplyForce));
	classDef.def("ApplyForce", static_cast<void (*)(lua_State *, pragma::physics::IRigidBody *, Vector3 &, Vector3 &)>(&ApplyForce));
	classDef.def("ApplyImpulse", static_cast<void (*)(lua_State *, pragma::physics::IRigidBody *, Vector3 &)>(&ApplyImpulse));
	classDef.def("ApplyImpulse", static_cast<void (*)(lua_State *, pragma::physics::IRigidBody *, Vector3 &, Vector3 &)>(&ApplyImpulse));
	classDef.def("ApplyTorque", &ApplyTorque);
	classDef.def("ApplyTorqueImpulse", &ApplyTorqueImpulse);
	classDef.def("ClearForces", &ClearForces);
	classDef.def("GetTotalForce", &GetTotalForce);
	classDef.def("GetTotalTorque", &GetTotalTorque);
	classDef.def("SetDamping", &SetDamping);
	classDef.def("SetLinearDamping", &SetLinearDamping);
	classDef.def("SetAngularDamping", &SetAngularDamping);
	classDef.def("GetLinearDamping", &GetLinearDamping);
	classDef.def("GetAngularDamping", &GetAngularDamping);
	classDef.def("GetBoneID", &GetBoneID);
	classDef.def("SetBoneID", &SetBoneID);
	classDef.def("SetLinearFactor", &SetLinearFactor);
	classDef.def("GetLinearFactor", &GetLinearFactor);
	classDef.def("SetAngularFactor", &SetAngularFactor);
	classDef.def("GetAngularFactor", &GetAngularFactor);
	classDef.def("SetLinearSleepingThreshold", &SetLinearSleepingThreshold);
	classDef.def("SetAngularSleepingThreshold", &SetAngularSleepingThreshold);
	classDef.def("SetSleepingThresholds", &SetSleepingThresholds);
	classDef.def("GetLinearSleepingThreshold", &GetLinearSleepingThreshold);
	classDef.def("GetAngularSleepingThreshold", &GetAngularSleepingThreshold);
	classDef.def("GetSleepingThreshold", &GetSleepingThreshold);
	classDef.def("GetCenterOfMassOffset", static_cast<void (*)(lua_State *, pragma::physics::IRigidBody *)>([](lua_State *l, pragma::physics::IRigidBody *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
			return;
		Lua::Push<Vector3>(l, hPhys->GetCenterOfMassOffset());
	}));
	classDef.def("SetCenterOfMassOffset", static_cast<void (*)(lua_State *, pragma::physics::IRigidBody *, const Vector3 &)>([](lua_State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &offset) {
		if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
			return;
		hPhys->SetCenterOfMassOffset(offset);
	}));
	classDef.def("SetKinematic", static_cast<void (*)(lua_State *, pragma::physics::IRigidBody *, bool)>([](lua_State *l, pragma::physics::IRigidBody *hPhys, bool bKinematic) {
		if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
			return;
		hPhys->SetKinematic(bKinematic);
	}));
	classDef.def("IsKinematic", static_cast<void (*)(lua_State *, pragma::physics::IRigidBody *)>([](lua_State *l, pragma::physics::IRigidBody *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
			return;
		Lua::PushBool(l, hPhys->IsKinematic());
	}));
	mod[classDef];
}
void Lua::PhysSoftBody::register_class(lua_State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::physics::ISoftBody, luabind::bases<pragma::physics::ICollisionObject, pragma::physics::IBase>>("SoftBody");
	classDef.def("IsValid", &IsValid);
	classDef.def("AddAeroForceToNode", &AddAeroForceToNode);
	classDef.def("AddAeroForceToFace", &AddAeroForceToFace);
	classDef.def("ApplyForce", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *&, uint32_t, const Vector3 &)>(&ApplyForce));
	classDef.def("ApplyForce", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *&, const Vector3 &)>(&ApplyForce));
	classDef.def("AddLinearVelocity", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *&, uint32_t, const Vector3 &)>(&AddLinearVelocity));
	classDef.def("AddLinearVelocity", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *&, const Vector3 &)>(&AddLinearVelocity));
	classDef.def("GetFriction", &GetFriction);
	classDef.def("GetHitFraction", &GetHitFraction);
	classDef.def("GetRollingFriction", &GetRollingFriction);
	classDef.def("GetAnisotropicFriction", &GetAnisotropicFriction);
	classDef.def("SetFriction", &SetFriction);
	classDef.def("SetHitFraction", &SetHitFraction);
	classDef.def("SetRollingFriction", &SetRollingFriction);
	classDef.def("SetAnisotropicFriction", &SetAnisotropicFriction);
	classDef.def("GetMass", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *&, uint32_t)>(&GetMass));
	classDef.def("GetMass", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *&)>(&GetMass));
	classDef.def("GetRestitution", &GetRestitution);
	classDef.def("GetRestLengthScale", &GetRestLengthScale);
	classDef.def("GetWindVelocity", &GetWindVelocity);
	classDef.def("SetMass", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *&, uint32_t, float)>(&SetMass));
	classDef.def("SetMass", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *&, float)>(&SetMass));
	classDef.def("SetRestitution", &SetRestitution);
	classDef.def("SetRestLengthScale", &SetRestLengthScale);
	classDef.def("SetWindVelocity", &SetWindVelocity);
	classDef.def("SetLinearVelocity", &SetLinearVelocity);
	classDef.def("SetVolumeDensity", &SetVolumeDensity);
	classDef.def("SetVolumeMass", &SetVolumeMass);
	classDef.def("GetVolume", &GetVolume);
	classDef.def("SetDensity", &SetDensity);

	classDef.def("SetAnchorsHardness", &SetAnchorsHardness);
	classDef.def("SetRigidContactsHardness", &SetRigidContactsHardness);
	classDef.def("SetDynamicFrictionCoefficient", &SetDynamicFrictionCoefficient);
	classDef.def("SetDragCoefficient", &SetDragCoefficient);
	classDef.def("SetDampingCoefficient", &SetDampingCoefficient);
	classDef.def("SetKineticContactsHardness", &SetKineticContactsHardness);
	classDef.def("SetLiftCoefficient", &SetLiftCoefficient);
	classDef.def("SetPoseMatchingCoefficient", &SetPoseMatchingCoefficient);
	classDef.def("SetPressureCoefficient", &SetPressureCoefficient);
	classDef.def("SetSoftContactsHardness", &SetSoftContactsHardness);
	classDef.def("SetSoftVsKineticHardness", &SetSoftVsKineticHardness);
	classDef.def("SetSoftVsRigidImpulseSplitK", &SetSoftVsRigidImpulseSplitK);
	classDef.def("SetSoftVsRigidHardness", &SetSoftVsRigidHardness);
	classDef.def("SetSoftVsRigidImpulseSplitR", &SetSoftVsRigidImpulseSplitR);
	classDef.def("SetSoftVsSoftHardness", &SetSoftVsSoftHardness);
	classDef.def("SetSoftVsRigidImpulseSplitS", &SetSoftVsRigidImpulseSplitS);
	classDef.def("SetVolumeConversationCoefficient", &SetVolumeConversationCoefficient);
	classDef.def("SetVelocitiesCorrectionFactor", &SetVelocitiesCorrectionFactor);
	classDef.def("GetAnchorsHardness", &GetAnchorsHardness);
	classDef.def("GetRigidContactsHardness", &GetRigidContactsHardness);
	classDef.def("GetDynamicFrictionCoefficient", &GetDynamicFrictionCoefficient);
	classDef.def("GetDragCoefficient", &GetDragCoefficient);
	classDef.def("GetDampingCoefficient", &GetDampingCoefficient);
	classDef.def("GetKineticContactsHardness", &GetKineticContactsHardness);
	classDef.def("GetLiftCoefficient", &GetLiftCoefficient);
	classDef.def("GetPoseMatchingCoefficient", &GetPoseMatchingCoefficient);
	classDef.def("GetPressureCoefficient", &GetPressureCoefficient);
	classDef.def("GetSoftContactsHardness", &GetSoftContactsHardness);
	classDef.def("GetSoftVsKineticHardness", &GetSoftVsKineticHardness);
	classDef.def("GetSoftVsRigidImpulseSplitK", &GetSoftVsRigidImpulseSplitK);
	classDef.def("GetSoftVsRigidHardness", &GetSoftVsRigidHardness);
	classDef.def("GetSoftVsRigidImpulseSplitR", &GetSoftVsRigidImpulseSplitR);
	classDef.def("GetSoftVsSoftHardness", &GetSoftVsSoftHardness);
	classDef.def("GetSoftVsRigidImpulseSplitS", &GetSoftVsRigidImpulseSplitS);
	classDef.def("GetVolumeConversationCoefficient", &GetVolumeConversationCoefficient);
	classDef.def("GetVelocitiesCorrectionFactor", &GetVelocitiesCorrectionFactor);
	classDef.def("SetMaterialAngularStiffnessCoefficient", &SetMaterialAngularStiffnessCoefficient);
	classDef.def("SetMaterialLinearStiffnessCoefficient", &SetMaterialLinearStiffnessCoefficient);
	classDef.def("SetMaterialVolumeStiffnessCoefficient", &SetMaterialVolumeStiffnessCoefficient);
	classDef.def("GetMaterialAngularStiffnessCoefficient", &GetMaterialAngularStiffnessCoefficient);
	classDef.def("GetMaterialLinearStiffnessCoefficient", &GetMaterialLinearStiffnessCoefficient);
	classDef.def("GetMaterialVolumeStiffnessCoefficient", &GetMaterialVolumeStiffnessCoefficient);
	classDef.def("AppendAnchor", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, const Vector3 &, bool, float)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, const Vector3 &, bool)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, const Vector3 &)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, bool, float)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, bool)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua_State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *)>(&AppendAnchor));
	classDef.def("GetNodeCount", &GetNodeCount);

	classDef.def("MeshVertexIndexToLocalVertexIndex", &MeshVertexIndexToLocalVertexIndex);
	classDef.def("LocalVertexIndexToMeshVertexIndex", &LocalVertexIndexToMeshVertexIndex);
	classDef.def("LocalVertexIndexToNodeIndex", &LocalVertexIndexToNodeIndex);
	classDef.def("NodeIndexToLocalVertexIndex", &NodeIndexToLocalVertexIndex);
	classDef.def("MeshVertexIndexToNodeIndex", &MeshVertexIndexToNodeIndex);
	classDef.def("NodeIndexToMeshVertexIndex", &NodeIndexToMeshVertexIndex);
	mod[classDef];
}

/////////////////////////////////////////////

void Lua::PhysCollisionObj::IsValid(lua_State *l, pragma::physics::ICollisionObject *hPhys) { Lua::PushBool(l, hPhys != nullptr); }
void Lua::PhysCollisionObj::Remove(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->ClaimOwnership().Remove();
}
void Lua::PhysCollisionObj::SetPos(lua_State *l, pragma::physics::ICollisionObject *hPhys, Vector3 &pos)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetPos(pos);
}
void Lua::PhysCollisionObj::GetPos(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetPos());
}
void Lua::PhysCollisionObj::GetBoundingSphere(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	auto *shape = hPhys->GetCollisionShape();
	if(shape == nullptr)
		return;
	Vector3 center;
	float radius;
	shape->GetBoundingSphere(center, radius);
	center += hPhys->GetPos();
	Lua::Push<Vector3>(l, center);
	Lua::PushNumber(l, radius);
}
void Lua::PhysCollisionObj::IsRigid(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::PushBool(l, hPhys->IsRigid());
}
void Lua::PhysCollisionObj::IsGhost(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::PushBool(l, hPhys->IsGhost());
}
void Lua::PhysCollisionObj::IsSoftBody(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::PushBool(l, hPhys->IsSoftBody());
}
void Lua::PhysCollisionObj::SetAngles(lua_State *l, pragma::physics::ICollisionObject *hPhys, const EulerAngles &ang)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetRotation(uquat::create(ang));
}
void Lua::PhysCollisionObj::GetAngles(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<EulerAngles>(l, EulerAngles(hPhys->GetRotation()));
}
void Lua::PhysCollisionObj::SetSimulationEnabled(lua_State *l, pragma::physics::ICollisionObject *hPhys, bool b)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetSimulationEnabled(b);
}
void Lua::PhysCollisionObj::DisableSimulation(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->DisableSimulation();
}
void Lua::PhysCollisionObj::EnableSimulation(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->EnableSimulation();
}
void Lua::PhysCollisionObj::IsSimulationEnabled(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::PushBool(l, hPhys->IsSimulationEnabled());
}
void Lua::PhysCollisionObj::GetShape(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	auto shape = hPhys->GetCollisionShape();
	if(shape == nullptr)
		return;
	shape->Push(l);
}
void Lua::PhysCollisionObj::GetActivationState(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::PushInt(l, hPhys->GetActivationState());
}
void Lua::PhysCollisionObj::SetActivationState(lua_State *l, pragma::physics::ICollisionObject *hPhys, int32_t state)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetActivationState(static_cast<pragma::physics::ICollisionObject::ActivationState>(state));
}
void Lua::PhysCollisionObj::Activate(lua_State *l, pragma::physics::ICollisionObject *hPhys, bool bForceActivation)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->WakeUp(bForceActivation);
}
void Lua::PhysCollisionObj::Activate(lua_State *l, pragma::physics::ICollisionObject *hPhys) { Activate(l, hPhys, false); }
void Lua::PhysCollisionObj::Spawn(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->Spawn();
}
void Lua::PhysCollisionObj::SetCollisionFilterGroup(lua_State *l, pragma::physics::ICollisionObject *hPhys, uint32_t group)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetCollisionFilterGroup(static_cast<CollisionMask>(group));
}
void Lua::PhysCollisionObj::GetCollisionFilterGroup(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::PushInt(l, hPhys->GetCollisionFilterGroup());
}
void Lua::PhysCollisionObj::SetCollisionFilterMask(lua_State *l, pragma::physics::ICollisionObject *hPhys, uint32_t mask)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetCollisionFilterMask(static_cast<CollisionMask>(mask));
}
void Lua::PhysCollisionObj::GetCollisionFilterMask(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::PushInt(l, hPhys->GetCollisionFilterMask());
}
void Lua::PhysCollisionObj::GetWorldTransform(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<umath::Transform>(l, hPhys->GetWorldTransform());
}
void Lua::PhysCollisionObj::SetWorldTransform(lua_State *l, pragma::physics::ICollisionObject *hPhys, const umath::Transform &t)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetWorldTransform(t);
}
void Lua::PhysCollisionObj::GetBounds(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Vector3 min, max;
	hPhys->GetAABB(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::PhysCollisionObj::SetRotation(lua_State *l, pragma::physics::ICollisionObject *hPhys, Quat &rot)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetRotation(rot);
}
void Lua::PhysCollisionObj::GetRotation(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<Quat>(l, hPhys->GetRotation());
}
void Lua::PhysCollisionObj::GetOrigin(lua_State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetOrigin());
}
void Lua::PhysCollisionObj::SetOrigin(lua_State *l, pragma::physics::ICollisionObject *hPhys, const Vector3 &origin)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetOrigin(origin);
}

/////////////////////////////////////////////

void Lua::PhysRigidBody::IsValid(lua_State *l, pragma::physics::IRigidBody *hPhys) { Lua::PushBool(l, hPhys != nullptr); }
void Lua::PhysRigidBody::SetLinearVelocity(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearVelocity(vel);
}
void Lua::PhysRigidBody::GetLinearVelocity(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetLinearVelocity());
}
void Lua::PhysRigidBody::SetAngularVelocity(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetAngularVelocity(vel);
}
void Lua::PhysRigidBody::GetAngularVelocity(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetAngularVelocity());
}
void Lua::PhysRigidBody::SetMass(lua_State *l, pragma::physics::IRigidBody *hPhys, float mass)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetMass(mass);
}
void Lua::PhysRigidBody::GetMass(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetMass());
}
void Lua::PhysRigidBody::GetInertia(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetInertia());
}
void Lua::PhysRigidBody::GetInvInertiaTensorWorld(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Mat3>(l, hPhys->GetInvInertiaTensorWorld());
}
void Lua::PhysRigidBody::ApplyForce(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyForce(force);
}
void Lua::PhysRigidBody::ApplyForce(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &force, Vector3 &relPos)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyForce(force, relPos);
}
void Lua::PhysRigidBody::ApplyImpulse(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &impulse)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyImpulse(impulse);
}
void Lua::PhysRigidBody::ApplyImpulse(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &impulse, Vector3 &relPos)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyImpulse(impulse, relPos);
}
void Lua::PhysRigidBody::ApplyTorque(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &torque)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyTorque(torque);
}
void Lua::PhysRigidBody::ApplyTorqueImpulse(lua_State *l, pragma::physics::IRigidBody *hPhys, Vector3 &torque)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyTorqueImpulse(torque);
}
void Lua::PhysRigidBody::ClearForces(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ClearForces();
}
void Lua::PhysRigidBody::GetTotalForce(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetTotalForce());
}
void Lua::PhysRigidBody::GetTotalTorque(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetTotalTorque());
}
void Lua::PhysRigidBody::SetDamping(lua_State *l, pragma::physics::IRigidBody *hPhys, float linDamping, float angDamping)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetDamping(linDamping, angDamping);
}
void Lua::PhysRigidBody::SetLinearDamping(lua_State *l, pragma::physics::IRigidBody *hPhys, float linDamping)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearDamping(linDamping);
}
void Lua::PhysRigidBody::SetAngularDamping(lua_State *l, pragma::physics::IRigidBody *hPhys, float angDamping)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetAngularDamping(angDamping);
}
void Lua::PhysRigidBody::GetLinearDamping(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetLinearDamping());
}
void Lua::PhysRigidBody::GetAngularDamping(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetAngularDamping());
}
void Lua::PhysRigidBody::GetBoneID(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetBoneID());
}
void Lua::PhysRigidBody::SetBoneID(lua_State *l, pragma::physics::IRigidBody *hPhys, UInt32 boneId)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetBoneID(boneId);
}
void Lua::PhysRigidBody::SetLinearFactor(lua_State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &factor)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearFactor(factor);
}
void Lua::PhysRigidBody::GetLinearFactor(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetLinearFactor());
}
void Lua::PhysRigidBody::SetAngularFactor(lua_State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &factor)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetAngularFactor(factor);
}
void Lua::PhysRigidBody::GetAngularFactor(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetAngularFactor());
}
void Lua::PhysRigidBody::SetLinearSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys, float threshold)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearSleepingThreshold(threshold);
}
void Lua::PhysRigidBody::SetAngularSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys, float threshold)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetAngularSleepingThreshold(threshold);
}
void Lua::PhysRigidBody::SetSleepingThresholds(lua_State *l, pragma::physics::IRigidBody *hPhys, float linear, float angular)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetSleepingThresholds(linear, angular);
}
void Lua::PhysRigidBody::GetLinearSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetLinearSleepingThreshold());
}
void Lua::PhysRigidBody::GetAngularSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetAngularSleepingThreshold());
}
void Lua::PhysRigidBody::GetSleepingThreshold(lua_State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	auto thresholds = hPhys->GetSleepingThreshold();
	Lua::PushNumber(l, thresholds.first);
	Lua::PushNumber(l, thresholds.second);
}

/////////////////////////////////////////////

void Lua::PhysSoftBody::IsValid(lua_State *l, pragma::physics::ISoftBody *hPhys) { Lua::PushBool(l, hPhys != nullptr); }
void Lua::PhysSoftBody::AddAeroForceToNode(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddAeroForceToNode(node, force);
}
void Lua::PhysSoftBody::AddAeroForceToFace(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t face, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddAeroForceToFace(face, force);
}
void Lua::PhysSoftBody::ApplyForce(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddForce(force);
}
void Lua::PhysSoftBody::ApplyForce(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddForce(node, force);
}
void Lua::PhysSoftBody::AddLinearVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddLinearVelocity(vel);
}
void Lua::PhysSoftBody::AddLinearVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddLinearVelocity(node, vel);
}
void Lua::PhysSoftBody::GetFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetFriction());
}
void Lua::PhysSoftBody::GetHitFraction(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetHitFraction());
}
void Lua::PhysSoftBody::GetRollingFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetRollingFriction());
}
void Lua::PhysSoftBody::GetAnisotropicFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetAnisotropicFriction());
}
void Lua::PhysSoftBody::SetFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys, float friction)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetFriction(friction);
}
void Lua::PhysSoftBody::SetHitFraction(lua_State *l, pragma::physics::ISoftBody *&hPhys, float fraction)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetHitFraction(fraction);
}
void Lua::PhysSoftBody::SetRollingFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys, float friction)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetRollingFriction(friction);
}
void Lua::PhysSoftBody::SetAnisotropicFriction(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &friction)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetAnisotropicFriction(friction);
}
void Lua::PhysSoftBody::GetMass(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetMass(node));
}
void Lua::PhysSoftBody::GetMass(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetMass());
}
void Lua::PhysSoftBody::GetRestitution(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetRestitution());
}
void Lua::PhysSoftBody::GetRestLengthScale(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetRestLengthScale());
}
void Lua::PhysSoftBody::GetWindVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetWindVelocity());
}
void Lua::PhysSoftBody::SetMass(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, float mass)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMass(node, mass);
}
void Lua::PhysSoftBody::SetMass(lua_State *l, pragma::physics::ISoftBody *&hPhys, float mass)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMass(mass);
}
void Lua::PhysSoftBody::SetRestitution(lua_State *l, pragma::physics::ISoftBody *&hPhys, float rest)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetRestitution(rest);
}
void Lua::PhysSoftBody::SetRestLengthScale(lua_State *l, pragma::physics::ISoftBody *&hPhys, float scale)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetRestLengthScale(scale);
}
void Lua::PhysSoftBody::SetWindVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetWindVelocity(vel);
}
void Lua::PhysSoftBody::SetLinearVelocity(lua_State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearVelocity(vel);
}
void Lua::PhysSoftBody::SetVolumeDensity(lua_State *l, pragma::physics::ISoftBody *&hPhys, float density)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetVolumeDensity(density);
}
void Lua::PhysSoftBody::SetVolumeMass(lua_State *l, pragma::physics::ISoftBody *&hPhys, float mass)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetVolumeMass(mass);
}
void Lua::PhysSoftBody::GetVolume(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetVolume());
}
void Lua::PhysSoftBody::SetDensity(lua_State *l, pragma::physics::ISoftBody *&hPhys, float density)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetDensity(density);
}

void Lua::PhysSoftBody::SetAnchorsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetAnchorsHardness(val);
}
void Lua::PhysSoftBody::SetRigidContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetRigidContactsHardness(val);
}
void Lua::PhysSoftBody::SetDynamicFrictionCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetDynamicFrictionCoefficient(val);
}
void Lua::PhysSoftBody::SetDragCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetDragCoefficient(val);
}
void Lua::PhysSoftBody::SetDampingCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetDampingCoefficient(val);
}
void Lua::PhysSoftBody::SetKineticContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetKineticContactsHardness(val);
}
void Lua::PhysSoftBody::SetLiftCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetLiftCoefficient(val);
}
void Lua::PhysSoftBody::SetPoseMatchingCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetPoseMatchingCoefficient(val);
}
void Lua::PhysSoftBody::SetPressureCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetPressureCoefficient(val);
}
void Lua::PhysSoftBody::SetSoftContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftContactsHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsKineticHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsKineticHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidImpulseSplitK(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsRigidImpulseSplitK(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsRigidHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidImpulseSplitR(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsRigidImpulseSplitR(val);
}
void Lua::PhysSoftBody::SetSoftVsSoftHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsSoftHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidImpulseSplitS(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsRigidImpulseSplitS(val);
}
void Lua::PhysSoftBody::SetVolumeConversationCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetVolumeConversationCoefficient(val);
}
void Lua::PhysSoftBody::SetVelocitiesCorrectionFactor(lua_State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetVelocitiesCorrectionFactor(val);
}

void Lua::PhysSoftBody::GetAnchorsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetAnchorsHardness());
}
void Lua::PhysSoftBody::GetRigidContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetRigidContactsHardness());
}
void Lua::PhysSoftBody::GetDynamicFrictionCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetDynamicFrictionCoefficient());
}
void Lua::PhysSoftBody::GetDragCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetDragCoefficient());
}
void Lua::PhysSoftBody::GetDampingCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetDampingCoefficient());
}
void Lua::PhysSoftBody::GetKineticContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetKineticContactsHardness());
}
void Lua::PhysSoftBody::GetLiftCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetLiftCoefficient());
}
void Lua::PhysSoftBody::GetPoseMatchingCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetPoseMatchingCoefficient());
}
void Lua::PhysSoftBody::GetPressureCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetPressureCoefficient());
}
void Lua::PhysSoftBody::GetSoftContactsHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetSoftContactsHardness());
}
void Lua::PhysSoftBody::GetSoftVsKineticHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetSoftVsKineticHardness());
}
void Lua::PhysSoftBody::GetSoftVsRigidImpulseSplitK(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetSoftVsRigidImpulseSplitK());
}
void Lua::PhysSoftBody::GetSoftVsRigidHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetSoftVsRigidHardness());
}
void Lua::PhysSoftBody::GetSoftVsRigidImpulseSplitR(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetSoftVsRigidImpulseSplitR());
}
void Lua::PhysSoftBody::GetSoftVsSoftHardness(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetSoftVsSoftHardness());
}
void Lua::PhysSoftBody::GetSoftVsRigidImpulseSplitS(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetSoftVsRigidImpulseSplitS());
}
void Lua::PhysSoftBody::GetVolumeConversationCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetVolumeConversationCoefficient());
}
void Lua::PhysSoftBody::GetVelocitiesCorrectionFactor(lua_State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetVelocitiesCorrectionFactor());
}
void Lua::PhysSoftBody::SetMaterialAngularStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMaterialAngularStiffnessCoefficient(matId, val);
}
void Lua::PhysSoftBody::SetMaterialLinearStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMaterialLinearStiffnessCoefficient(matId, val);
}
void Lua::PhysSoftBody::SetMaterialVolumeStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMaterialVolumeStiffnessCoefficient(matId, val);
}
void Lua::PhysSoftBody::GetMaterialAngularStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetMaterialAngularStiffnessCoefficient(matId));
}
void Lua::PhysSoftBody::GetMaterialLinearStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetMaterialLinearStiffnessCoefficient(matId));
}
void Lua::PhysSoftBody::GetMaterialVolumeStiffnessCoefficient(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushNumber(l, hPhys->GetMaterialVolumeStiffnessCoefficient(matId));
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot, bool bDisableCollision, float influence)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, localPivot, bDisableCollision, influence);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot, bool bDisableCollision)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, localPivot, bDisableCollision);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, localPivot);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, bool bDisableCollision, float influence)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, bDisableCollision, influence);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, bool bDisableCollision)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, bDisableCollision);
}
void Lua::PhysSoftBody::AppendAnchor(lua_State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody);
}
void Lua::PhysSoftBody::GetNodeCount(lua_State *l, pragma::physics::ISoftBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::PushInt(l, hPhys->GetNodeCount());
}
void Lua::PhysSoftBody::MeshVertexIndexToLocalVertexIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t meshVertexIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t localIndex = 0u;
	if(hPhys->MeshVertexIndexToLocalVertexIndex(meshVertexIndex, localIndex) == false)
		return;
	Lua::PushInt(l, localIndex);
}
void Lua::PhysSoftBody::LocalVertexIndexToMeshVertexIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t localIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t meshVertexIndex = 0u;
	if(hPhys->LocalVertexIndexToMeshVertexIndex(localIndex, meshVertexIndex) == false)
		return;
	Lua::PushInt(l, meshVertexIndex);
}
void Lua::PhysSoftBody::LocalVertexIndexToNodeIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t localVertexIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t nodeIndex = 0u;
	if(hPhys->LocalVertexIndexToNodeIndex(localVertexIndex, nodeIndex) == false)
		return;
	Lua::PushInt(l, nodeIndex);
}
void Lua::PhysSoftBody::NodeIndexToLocalVertexIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t nodeIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t localIndex = 0u;
	if(hPhys->NodeIndexToLocalVertexIndex(nodeIndex, localIndex) == false)
		return;
	Lua::PushInt(l, localIndex);
}

void Lua::PhysSoftBody::MeshVertexIndexToNodeIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t meshVertexIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t nodeIndex = 0u;
	if(hPhys->MeshVertexIndexToNodeIndex(meshVertexIndex, nodeIndex) == false)
		return;
	Lua::PushInt(l, nodeIndex);
}
void Lua::PhysSoftBody::NodeIndexToMeshVertexIndex(lua_State *l, pragma::physics::ISoftBody *&hPhys, uint32_t nodeIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t meshIndex = 0u;
	if(hPhys->NodeIndexToMeshVertexIndex(nodeIndex, meshIndex) == false)
		return;
	Lua::PushInt(l, meshIndex);
}

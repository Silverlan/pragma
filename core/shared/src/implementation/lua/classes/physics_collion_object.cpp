// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.shared;

import :scripting.lua.classes.physics;

namespace Lua {
	namespace PhysCollisionObj {
		static void IsValid(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void Remove(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetPos(lua::State *l, pragma::physics::ICollisionObject *hPhys, Vector3 &pos);
		static void GetPos(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetRotation(lua::State *l, pragma::physics::ICollisionObject *hPhys, Quat &rot);
		static void GetRotation(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetOrigin(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetOrigin(lua::State *l, pragma::physics::ICollisionObject *hPhys, const Vector3 &origin);
		static void GetBounds(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetBoundingSphere(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void IsRigid(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void IsGhost(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void IsSoftBody(lua::State *l, pragma::physics::ICollisionObject *hPhys);

		static void SetAngles(lua::State *l, pragma::physics::ICollisionObject *hPhys, const EulerAngles &ang);
		static void GetAngles(lua::State *l, pragma::physics::ICollisionObject *hPhys);

		static void SetSimulationEnabled(lua::State *l, pragma::physics::ICollisionObject *hPhys, bool b);
		static void DisableSimulation(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void EnableSimulation(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void IsSimulationEnabled(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetShape(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetActivationState(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetActivationState(lua::State *l, pragma::physics::ICollisionObject *hPhys, int32_t state);
		static void Activate(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void Activate(lua::State *l, pragma::physics::ICollisionObject *hPhys, bool bForceActivation);
		static void Spawn(lua::State *l, pragma::physics::ICollisionObject *hPhys);

		static void SetCollisionFilterGroup(lua::State *l, pragma::physics::ICollisionObject *hPhys, uint32_t group);
		static void GetCollisionFilterGroup(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetCollisionFilterMask(lua::State *l, pragma::physics::ICollisionObject *hPhys, uint32_t mask);
		static void GetCollisionFilterMask(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void GetWorldTransform(lua::State *l, pragma::physics::ICollisionObject *hPhys);
		static void SetWorldTransform(lua::State *l, pragma::physics::ICollisionObject *hPhys, const pragma::math::Transform &t);
	};
	namespace PhysRigidBody {
		static void register_class(lua::State *l, luabind::module_ &mod);
		static void IsValid(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void SetLinearVelocity(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &vel);
		static void GetLinearVelocity(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void SetAngularVelocity(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &vel);
		static void GetAngularVelocity(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void SetMass(lua::State *l, pragma::physics::IRigidBody *hPhys, float mass);
		static void GetMass(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void GetInertia(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void GetInvInertiaTensorWorld(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void ApplyForce(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &force);
		static void ApplyForce(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &force, Vector3 &relPos);
		static void ApplyImpulse(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &impulse);
		static void ApplyImpulse(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &impulse, Vector3 &relPos);
		static void ApplyTorque(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &torque);
		static void ApplyTorqueImpulse(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &torque);
		static void ClearForces(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void GetTotalForce(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void GetTotalTorque(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void GetBoneID(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void SetBoneID(lua::State *l, pragma::physics::IRigidBody *hPhys, UInt32 boneId);

		static void SetDamping(lua::State *l, pragma::physics::IRigidBody *hPhys, float linDamping, float angDamping);
		static void SetLinearDamping(lua::State *l, pragma::physics::IRigidBody *hPhys, float linDamping);
		static void SetAngularDamping(lua::State *l, pragma::physics::IRigidBody *hPhys, float angDamping);
		static void GetLinearDamping(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void GetAngularDamping(lua::State *l, pragma::physics::IRigidBody *hPhys);

		static void SetLinearFactor(lua::State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &factor);
		static void GetLinearFactor(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void SetAngularFactor(lua::State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &factor);
		static void GetAngularFactor(lua::State *l, pragma::physics::IRigidBody *hPhys);

		static void SetLinearSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys, float threshold);
		static void SetAngularSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys, float threshold);
		static void SetSleepingThresholds(lua::State *l, pragma::physics::IRigidBody *hPhys, float linear, float angular);
		static void GetLinearSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void GetAngularSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys);
		static void GetSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys);
	};
	namespace PhysSoftBody {
		static void register_class(lua::State *l, luabind::module_ &mod);
		static void IsValid(lua::State *l, pragma::physics::ISoftBody *hPhys);
		static void AddAeroForceToNode(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &force);
		static void AddAeroForceToFace(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t face, const Vector3 &force);
		static void ApplyForce(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &force);
		static void ApplyForce(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &force);
		static void AddLinearVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel);
		static void AddLinearVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &vel);
		static void GetFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetHitFraction(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetRollingFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetAnisotropicFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void SetFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys, float friction);
		static void SetHitFraction(lua::State *l, pragma::physics::ISoftBody *&hPhys, float fraction);
		static void SetRollingFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys, float friction);
		static void SetAnisotropicFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &friction);
		static void GetMass(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node);
		static void GetMass(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetRestitution(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetRestLengthScale(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetWindVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void SetMass(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, float mass);
		static void SetMass(lua::State *l, pragma::physics::ISoftBody *&hPhys, float mass);
		static void SetRestitution(lua::State *l, pragma::physics::ISoftBody *&hPhys, float rest);
		static void SetRestLengthScale(lua::State *l, pragma::physics::ISoftBody *&hPhys, float scale);
		static void SetWindVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel);
		static void SetLinearVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel);
		static void SetVolumeDensity(lua::State *l, pragma::physics::ISoftBody *&hPhys, float density);
		static void SetVolumeMass(lua::State *l, pragma::physics::ISoftBody *&hPhys, float mass);
		static void GetVolume(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void SetDensity(lua::State *l, pragma::physics::ISoftBody *&hPhys, float density);

		static void SetAnchorsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetRigidContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetDynamicFrictionCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetDragCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetDampingCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetKineticContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetLiftCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetPoseMatchingCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetPressureCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsKineticHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsRigidImpulseSplitK(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsRigidHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsRigidImpulseSplitR(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsSoftHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetSoftVsRigidImpulseSplitS(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetVolumeConversationCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);
		static void SetVelocitiesCorrectionFactor(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val);

		static void GetAnchorsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetRigidContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetDynamicFrictionCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetDragCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetDampingCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetKineticContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetLiftCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetPoseMatchingCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetPressureCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsKineticHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsRigidImpulseSplitK(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsRigidHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsRigidImpulseSplitR(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsSoftHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetSoftVsRigidImpulseSplitS(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetVolumeConversationCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys);
		static void GetVelocitiesCorrectionFactor(lua::State *l, pragma::physics::ISoftBody *&hPhys);

		static void SetMaterialAngularStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val);
		static void SetMaterialLinearStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val);
		static void SetMaterialVolumeStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val);
		static void GetMaterialAngularStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId);
		static void GetMaterialLinearStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId);
		static void GetMaterialVolumeStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId);

		static void AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot, bool bDisableCollision, float influence);
		static void AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot, bool bDisableCollision);
		static void AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot);
		static void AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, bool bDisableCollision, float influence);
		static void AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, bool bDisableCollision);
		static void AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody);
		static void GetNodeCount(lua::State *l, pragma::physics::ISoftBody *hPhys);

		static void MeshVertexIndexToLocalVertexIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t meshVertexIndex);
		static void LocalVertexIndexToMeshVertexIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t localIndex);
		static void LocalVertexIndexToNodeIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t localVertexIndex);
		static void NodeIndexToLocalVertexIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t nodeIndex);

		static void MeshVertexIndexToNodeIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t meshVertexIndex);
		static void NodeIndexToMeshVertexIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t nodeIndex);
	};
};

// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::physics, ICollisionObject);

void Lua::PhysCollisionObj::register_class(lua::State *l, luabind::module_ &mod)
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
	classDef.def("WakeUp", static_cast<void (*)(lua::State *, pragma::physics::ICollisionObject *, bool)>(&Activate));
	classDef.def("WakeUp", static_cast<void (*)(lua::State *, pragma::physics::ICollisionObject *)>(&Activate));
	classDef.def("Spawn", &Spawn);
	classDef.def("SetCollisionFilterGroup", &SetCollisionFilterGroup);
	classDef.def("GetCollisionFilterGroup", &GetCollisionFilterGroup);
	classDef.def("SetCollisionFilterMask", &SetCollisionFilterMask);
	classDef.def("GetCollisionFilterMask", &GetCollisionFilterMask);
	classDef.def("GetWorldTransform", &GetWorldTransform);
	classDef.def("SetWorldTransform", &SetWorldTransform);
	classDef.def("SetAngles", &SetAngles);
	classDef.def("GetAngles", &GetAngles);

	classDef.def("PutToSleep", static_cast<void (*)(lua::State *, pragma::physics::ICollisionObject *)>([](lua::State *l, pragma::physics::ICollisionObject *hPhys) {
		if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
			return;
		hPhys->PutToSleep();
	}));
	classDef.def("IsAsleep", static_cast<void (*)(lua::State *, pragma::physics::ICollisionObject *)>([](lua::State *l, pragma::physics::ICollisionObject *hPhys) {
		if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
			return;
		PushBool(l, hPhys->IsAsleep());
	}));
	classDef.def("SetAlwaysAwake", static_cast<void (*)(lua::State *, pragma::physics::ICollisionObject *, bool)>([](lua::State *l, pragma::physics::ICollisionObject *hPhys, bool alwaysAwake) {
		if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
			return;
		hPhys->SetAlwaysAwake(alwaysAwake);
	}));
	classDef.def("IsAlwaysAwake", static_cast<bool (*)(lua::State *, pragma::physics::ICollisionObject *)>([](lua::State *l, pragma::physics::ICollisionObject *hPhys) -> bool {
		if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
			return false;
		return hPhys->IsAlwaysAwake();
	}));

	mod[classDef];

	PhysRigidBody::register_class(l, mod);
	PhysSoftBody::register_class(l, mod);

	auto classDefGhost = luabind::class_<pragma::physics::IGhostObject, pragma::physics::IBase>("GhostObj");
	classDefGhost.def("IsValid", static_cast<void (*)(lua::State *, pragma::physics::IGhostObject *)>([](lua::State *l, pragma::physics::IGhostObject *hPhys) { PushBool(l, hPhys != nullptr); }));
	mod[classDefGhost];
}

void Lua::PhysRigidBody::register_class(lua::State *l, luabind::module_ &mod)
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
	classDef.def("ApplyForce", static_cast<void (*)(lua::State *, pragma::physics::IRigidBody *, Vector3 &)>(&ApplyForce));
	classDef.def("ApplyForce", static_cast<void (*)(lua::State *, pragma::physics::IRigidBody *, Vector3 &, Vector3 &)>(&ApplyForce));
	classDef.def("ApplyImpulse", static_cast<void (*)(lua::State *, pragma::physics::IRigidBody *, Vector3 &)>(&ApplyImpulse));
	classDef.def("ApplyImpulse", static_cast<void (*)(lua::State *, pragma::physics::IRigidBody *, Vector3 &, Vector3 &)>(&ApplyImpulse));
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
	classDef.def("GetCenterOfMassOffset", static_cast<void (*)(lua::State *, pragma::physics::IRigidBody *)>([](lua::State *l, pragma::physics::IRigidBody *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
			return;
		Lua::Push<Vector3>(l, hPhys->GetCenterOfMassOffset());
	}));
	classDef.def("SetCenterOfMassOffset", static_cast<void (*)(lua::State *, pragma::physics::IRigidBody *, const Vector3 &)>([](lua::State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &offset) {
		if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
			return;
		hPhys->SetCenterOfMassOffset(offset);
	}));
	classDef.def("SetKinematic", static_cast<void (*)(lua::State *, pragma::physics::IRigidBody *, bool)>([](lua::State *l, pragma::physics::IRigidBody *hPhys, bool bKinematic) {
		if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
			return;
		hPhys->SetKinematic(bKinematic);
	}));
	classDef.def("IsKinematic", static_cast<void (*)(lua::State *, pragma::physics::IRigidBody *)>([](lua::State *l, pragma::physics::IRigidBody *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
			return;
		PushBool(l, hPhys->IsKinematic());
	}));
	mod[classDef];
}
void Lua::PhysSoftBody::register_class(lua::State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::physics::ISoftBody, luabind::bases<pragma::physics::ICollisionObject, pragma::physics::IBase>>("SoftBody");
	classDef.def("IsValid", &IsValid);
	classDef.def("AddAeroForceToNode", &AddAeroForceToNode);
	classDef.def("AddAeroForceToFace", &AddAeroForceToFace);
	classDef.def("ApplyForce", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *&, uint32_t, const Vector3 &)>(&ApplyForce));
	classDef.def("ApplyForce", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *&, const Vector3 &)>(&ApplyForce));
	classDef.def("AddLinearVelocity", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *&, uint32_t, const Vector3 &)>(&AddLinearVelocity));
	classDef.def("AddLinearVelocity", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *&, const Vector3 &)>(&AddLinearVelocity));
	classDef.def("GetFriction", &GetFriction);
	classDef.def("GetHitFraction", &GetHitFraction);
	classDef.def("GetRollingFriction", &GetRollingFriction);
	classDef.def("GetAnisotropicFriction", &GetAnisotropicFriction);
	classDef.def("SetFriction", &SetFriction);
	classDef.def("SetHitFraction", &SetHitFraction);
	classDef.def("SetRollingFriction", &SetRollingFriction);
	classDef.def("SetAnisotropicFriction", &SetAnisotropicFriction);
	classDef.def("GetMass", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *&, uint32_t)>(&GetMass));
	classDef.def("GetMass", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *&)>(&GetMass));
	classDef.def("GetRestitution", &GetRestitution);
	classDef.def("GetRestLengthScale", &GetRestLengthScale);
	classDef.def("GetWindVelocity", &GetWindVelocity);
	classDef.def("SetMass", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *&, uint32_t, float)>(&SetMass));
	classDef.def("SetMass", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *&, float)>(&SetMass));
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
	classDef.def("AppendAnchor", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, const Vector3 &, bool, float)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, const Vector3 &, bool)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, const Vector3 &)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, bool, float)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *, bool)>(&AppendAnchor));
	classDef.def("AppendAnchor", static_cast<void (*)(lua::State *, pragma::physics::ISoftBody *, uint32_t, pragma::physics::IRigidBody *)>(&AppendAnchor));
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

void Lua::PhysCollisionObj::IsValid(lua::State *l, pragma::physics::ICollisionObject *hPhys) { PushBool(l, hPhys != nullptr); }
void Lua::PhysCollisionObj::Remove(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->ClaimOwnership().Remove();
}
void Lua::PhysCollisionObj::SetPos(lua::State *l, pragma::physics::ICollisionObject *hPhys, Vector3 &pos)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetPos(pos);
}
void Lua::PhysCollisionObj::GetPos(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetPos());
}
void Lua::PhysCollisionObj::GetBoundingSphere(lua::State *l, pragma::physics::ICollisionObject *hPhys)
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
	PushNumber(l, radius);
}
void Lua::PhysCollisionObj::IsRigid(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	PushBool(l, hPhys->IsRigid());
}
void Lua::PhysCollisionObj::IsGhost(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	PushBool(l, hPhys->IsGhost());
}
void Lua::PhysCollisionObj::IsSoftBody(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	PushBool(l, hPhys->IsSoftBody());
}
void Lua::PhysCollisionObj::SetAngles(lua::State *l, pragma::physics::ICollisionObject *hPhys, const EulerAngles &ang)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetRotation(uquat::create(ang));
}
void Lua::PhysCollisionObj::GetAngles(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<EulerAngles>(l, EulerAngles(hPhys->GetRotation()));
}
void Lua::PhysCollisionObj::SetSimulationEnabled(lua::State *l, pragma::physics::ICollisionObject *hPhys, bool b)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetSimulationEnabled(b);
}
void Lua::PhysCollisionObj::DisableSimulation(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->DisableSimulation();
}
void Lua::PhysCollisionObj::EnableSimulation(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->EnableSimulation();
}
void Lua::PhysCollisionObj::IsSimulationEnabled(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	PushBool(l, hPhys->IsSimulationEnabled());
}
void Lua::PhysCollisionObj::GetShape(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	auto shape = hPhys->GetCollisionShape();
	if(shape == nullptr)
		return;
	shape->Push(l);
}
void Lua::PhysCollisionObj::GetActivationState(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	PushInt(l, hPhys->GetActivationState());
}
void Lua::PhysCollisionObj::SetActivationState(lua::State *l, pragma::physics::ICollisionObject *hPhys, int32_t state)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetActivationState(static_cast<pragma::physics::ICollisionObject::ActivationState>(state));
}
void Lua::PhysCollisionObj::Activate(lua::State *l, pragma::physics::ICollisionObject *hPhys, bool bForceActivation)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->WakeUp(bForceActivation);
}
void Lua::PhysCollisionObj::Activate(lua::State *l, pragma::physics::ICollisionObject *hPhys) { Activate(l, hPhys, false); }
void Lua::PhysCollisionObj::Spawn(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->Spawn();
}
void Lua::PhysCollisionObj::SetCollisionFilterGroup(lua::State *l, pragma::physics::ICollisionObject *hPhys, uint32_t group)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetCollisionFilterGroup(static_cast<pragma::physics::CollisionMask>(group));
}
void Lua::PhysCollisionObj::GetCollisionFilterGroup(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	PushInt(l, hPhys->GetCollisionFilterGroup());
}
void Lua::PhysCollisionObj::SetCollisionFilterMask(lua::State *l, pragma::physics::ICollisionObject *hPhys, uint32_t mask)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetCollisionFilterMask(static_cast<pragma::physics::CollisionMask>(mask));
}
void Lua::PhysCollisionObj::GetCollisionFilterMask(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	PushInt(l, hPhys->GetCollisionFilterMask());
}
void Lua::PhysCollisionObj::GetWorldTransform(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<pragma::math::Transform>(l, hPhys->GetWorldTransform());
}
void Lua::PhysCollisionObj::SetWorldTransform(lua::State *l, pragma::physics::ICollisionObject *hPhys, const pragma::math::Transform &t)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetWorldTransform(t);
}
void Lua::PhysCollisionObj::GetBounds(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Vector3 min, max;
	hPhys->GetAABB(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::PhysCollisionObj::SetRotation(lua::State *l, pragma::physics::ICollisionObject *hPhys, Quat &rot)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetRotation(rot);
}
void Lua::PhysCollisionObj::GetRotation(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<Quat>(l, hPhys->GetRotation());
}
void Lua::PhysCollisionObj::GetOrigin(lua::State *l, pragma::physics::ICollisionObject *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetOrigin());
}
void Lua::PhysCollisionObj::SetOrigin(lua::State *l, pragma::physics::ICollisionObject *hPhys, const Vector3 &origin)
{
	if(Lua::CheckHandle<pragma::physics::ICollisionObject>(l, hPhys) == false)
		return;
	hPhys->SetOrigin(origin);
}

/////////////////////////////////////////////

void Lua::PhysRigidBody::IsValid(lua::State *l, pragma::physics::IRigidBody *hPhys) { PushBool(l, hPhys != nullptr); }
void Lua::PhysRigidBody::SetLinearVelocity(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearVelocity(vel);
}
void Lua::PhysRigidBody::GetLinearVelocity(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetLinearVelocity());
}
void Lua::PhysRigidBody::SetAngularVelocity(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetAngularVelocity(vel);
}
void Lua::PhysRigidBody::GetAngularVelocity(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetAngularVelocity());
}
void Lua::PhysRigidBody::SetMass(lua::State *l, pragma::physics::IRigidBody *hPhys, float mass)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetMass(mass);
}
void Lua::PhysRigidBody::GetMass(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetMass());
}
void Lua::PhysRigidBody::GetInertia(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetInertia());
}
void Lua::PhysRigidBody::GetInvInertiaTensorWorld(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<::Mat3>(l, hPhys->GetInvInertiaTensorWorld());
}
void Lua::PhysRigidBody::ApplyForce(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyForce(force);
}
void Lua::PhysRigidBody::ApplyForce(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &force, Vector3 &relPos)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyForce(force, relPos);
}
void Lua::PhysRigidBody::ApplyImpulse(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &impulse)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyImpulse(impulse);
}
void Lua::PhysRigidBody::ApplyImpulse(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &impulse, Vector3 &relPos)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyImpulse(impulse, relPos);
}
void Lua::PhysRigidBody::ApplyTorque(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &torque)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyTorque(torque);
}
void Lua::PhysRigidBody::ApplyTorqueImpulse(lua::State *l, pragma::physics::IRigidBody *hPhys, Vector3 &torque)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ApplyTorqueImpulse(torque);
}
void Lua::PhysRigidBody::ClearForces(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->ClearForces();
}
void Lua::PhysRigidBody::GetTotalForce(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetTotalForce());
}
void Lua::PhysRigidBody::GetTotalTorque(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetTotalTorque());
}
void Lua::PhysRigidBody::SetDamping(lua::State *l, pragma::physics::IRigidBody *hPhys, float linDamping, float angDamping)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetDamping(linDamping, angDamping);
}
void Lua::PhysRigidBody::SetLinearDamping(lua::State *l, pragma::physics::IRigidBody *hPhys, float linDamping)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearDamping(linDamping);
}
void Lua::PhysRigidBody::SetAngularDamping(lua::State *l, pragma::physics::IRigidBody *hPhys, float angDamping)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetAngularDamping(angDamping);
}
void Lua::PhysRigidBody::GetLinearDamping(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetLinearDamping());
}
void Lua::PhysRigidBody::GetAngularDamping(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetAngularDamping());
}
void Lua::PhysRigidBody::GetBoneID(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetBoneID());
}
void Lua::PhysRigidBody::SetBoneID(lua::State *l, pragma::physics::IRigidBody *hPhys, UInt32 boneId)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetBoneID(boneId);
}
void Lua::PhysRigidBody::SetLinearFactor(lua::State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &factor)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearFactor(factor);
}
void Lua::PhysRigidBody::GetLinearFactor(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetLinearFactor());
}
void Lua::PhysRigidBody::SetAngularFactor(lua::State *l, pragma::physics::IRigidBody *hPhys, const Vector3 &factor)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetAngularFactor(factor);
}
void Lua::PhysRigidBody::GetAngularFactor(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetAngularFactor());
}
void Lua::PhysRigidBody::SetLinearSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys, float threshold)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearSleepingThreshold(threshold);
}
void Lua::PhysRigidBody::SetAngularSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys, float threshold)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetAngularSleepingThreshold(threshold);
}
void Lua::PhysRigidBody::SetSleepingThresholds(lua::State *l, pragma::physics::IRigidBody *hPhys, float linear, float angular)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	hPhys->SetSleepingThresholds(linear, angular);
}
void Lua::PhysRigidBody::GetLinearSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetLinearSleepingThreshold());
}
void Lua::PhysRigidBody::GetAngularSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetAngularSleepingThreshold());
}
void Lua::PhysRigidBody::GetSleepingThreshold(lua::State *l, pragma::physics::IRigidBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::IRigidBody>(l, hPhys) == false)
		return;
	auto thresholds = hPhys->GetSleepingThreshold();
	PushNumber(l, thresholds.first);
	PushNumber(l, thresholds.second);
}

/////////////////////////////////////////////

void Lua::PhysSoftBody::IsValid(lua::State *l, pragma::physics::ISoftBody *hPhys) { PushBool(l, hPhys != nullptr); }
void Lua::PhysSoftBody::AddAeroForceToNode(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddAeroForceToNode(node, force);
}
void Lua::PhysSoftBody::AddAeroForceToFace(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t face, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddAeroForceToFace(face, force);
}
void Lua::PhysSoftBody::ApplyForce(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddForce(force);
}
void Lua::PhysSoftBody::ApplyForce(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddForce(node, force);
}
void Lua::PhysSoftBody::AddLinearVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddLinearVelocity(vel);
}
void Lua::PhysSoftBody::AddLinearVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, const Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->AddLinearVelocity(node, vel);
}
void Lua::PhysSoftBody::GetFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetFriction());
}
void Lua::PhysSoftBody::GetHitFraction(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetHitFraction());
}
void Lua::PhysSoftBody::GetRollingFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetRollingFriction());
}
void Lua::PhysSoftBody::GetAnisotropicFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetAnisotropicFriction());
}
void Lua::PhysSoftBody::SetFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys, float friction)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetFriction(friction);
}
void Lua::PhysSoftBody::SetHitFraction(lua::State *l, pragma::physics::ISoftBody *&hPhys, float fraction)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetHitFraction(fraction);
}
void Lua::PhysSoftBody::SetRollingFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys, float friction)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetRollingFriction(friction);
}
void Lua::PhysSoftBody::SetAnisotropicFriction(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &friction)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetAnisotropicFriction(friction);
}
void Lua::PhysSoftBody::GetMass(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetMass(node));
}
void Lua::PhysSoftBody::GetMass(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetMass());
}
void Lua::PhysSoftBody::GetRestitution(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetRestitution());
}
void Lua::PhysSoftBody::GetRestLengthScale(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetRestLengthScale());
}
void Lua::PhysSoftBody::GetWindVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	Lua::Push<Vector3>(l, hPhys->GetWindVelocity());
}
void Lua::PhysSoftBody::SetMass(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t node, float mass)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMass(node, mass);
}
void Lua::PhysSoftBody::SetMass(lua::State *l, pragma::physics::ISoftBody *&hPhys, float mass)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMass(mass);
}
void Lua::PhysSoftBody::SetRestitution(lua::State *l, pragma::physics::ISoftBody *&hPhys, float rest)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetRestitution(rest);
}
void Lua::PhysSoftBody::SetRestLengthScale(lua::State *l, pragma::physics::ISoftBody *&hPhys, float scale)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetRestLengthScale(scale);
}
void Lua::PhysSoftBody::SetWindVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetWindVelocity(vel);
}
void Lua::PhysSoftBody::SetLinearVelocity(lua::State *l, pragma::physics::ISoftBody *&hPhys, const Vector3 &vel)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetLinearVelocity(vel);
}
void Lua::PhysSoftBody::SetVolumeDensity(lua::State *l, pragma::physics::ISoftBody *&hPhys, float density)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetVolumeDensity(density);
}
void Lua::PhysSoftBody::SetVolumeMass(lua::State *l, pragma::physics::ISoftBody *&hPhys, float mass)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetVolumeMass(mass);
}
void Lua::PhysSoftBody::GetVolume(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetVolume());
}
void Lua::PhysSoftBody::SetDensity(lua::State *l, pragma::physics::ISoftBody *&hPhys, float density)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetDensity(density);
}

void Lua::PhysSoftBody::SetAnchorsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetAnchorsHardness(val);
}
void Lua::PhysSoftBody::SetRigidContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetRigidContactsHardness(val);
}
void Lua::PhysSoftBody::SetDynamicFrictionCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetDynamicFrictionCoefficient(val);
}
void Lua::PhysSoftBody::SetDragCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetDragCoefficient(val);
}
void Lua::PhysSoftBody::SetDampingCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetDampingCoefficient(val);
}
void Lua::PhysSoftBody::SetKineticContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetKineticContactsHardness(val);
}
void Lua::PhysSoftBody::SetLiftCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetLiftCoefficient(val);
}
void Lua::PhysSoftBody::SetPoseMatchingCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetPoseMatchingCoefficient(val);
}
void Lua::PhysSoftBody::SetPressureCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetPressureCoefficient(val);
}
void Lua::PhysSoftBody::SetSoftContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftContactsHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsKineticHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsKineticHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidImpulseSplitK(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsRigidImpulseSplitK(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsRigidHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidImpulseSplitR(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsRigidImpulseSplitR(val);
}
void Lua::PhysSoftBody::SetSoftVsSoftHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsSoftHardness(val);
}
void Lua::PhysSoftBody::SetSoftVsRigidImpulseSplitS(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetSoftVsRigidImpulseSplitS(val);
}
void Lua::PhysSoftBody::SetVolumeConversationCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetVolumeConversationCoefficient(val);
}
void Lua::PhysSoftBody::SetVelocitiesCorrectionFactor(lua::State *l, pragma::physics::ISoftBody *&hPhys, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetVelocitiesCorrectionFactor(val);
}

void Lua::PhysSoftBody::GetAnchorsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetAnchorsHardness());
}
void Lua::PhysSoftBody::GetRigidContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetRigidContactsHardness());
}
void Lua::PhysSoftBody::GetDynamicFrictionCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetDynamicFrictionCoefficient());
}
void Lua::PhysSoftBody::GetDragCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetDragCoefficient());
}
void Lua::PhysSoftBody::GetDampingCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetDampingCoefficient());
}
void Lua::PhysSoftBody::GetKineticContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetKineticContactsHardness());
}
void Lua::PhysSoftBody::GetLiftCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetLiftCoefficient());
}
void Lua::PhysSoftBody::GetPoseMatchingCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetPoseMatchingCoefficient());
}
void Lua::PhysSoftBody::GetPressureCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetPressureCoefficient());
}
void Lua::PhysSoftBody::GetSoftContactsHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetSoftContactsHardness());
}
void Lua::PhysSoftBody::GetSoftVsKineticHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetSoftVsKineticHardness());
}
void Lua::PhysSoftBody::GetSoftVsRigidImpulseSplitK(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetSoftVsRigidImpulseSplitK());
}
void Lua::PhysSoftBody::GetSoftVsRigidHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetSoftVsRigidHardness());
}
void Lua::PhysSoftBody::GetSoftVsRigidImpulseSplitR(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetSoftVsRigidImpulseSplitR());
}
void Lua::PhysSoftBody::GetSoftVsSoftHardness(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetSoftVsSoftHardness());
}
void Lua::PhysSoftBody::GetSoftVsRigidImpulseSplitS(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetSoftVsRigidImpulseSplitS());
}
void Lua::PhysSoftBody::GetVolumeConversationCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetVolumeConversationCoefficient());
}
void Lua::PhysSoftBody::GetVelocitiesCorrectionFactor(lua::State *l, pragma::physics::ISoftBody *&hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetVelocitiesCorrectionFactor());
}
void Lua::PhysSoftBody::SetMaterialAngularStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMaterialAngularStiffnessCoefficient(matId, val);
}
void Lua::PhysSoftBody::SetMaterialLinearStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMaterialLinearStiffnessCoefficient(matId, val);
}
void Lua::PhysSoftBody::SetMaterialVolumeStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId, float val)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	hPhys->SetMaterialVolumeStiffnessCoefficient(matId, val);
}
void Lua::PhysSoftBody::GetMaterialAngularStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetMaterialAngularStiffnessCoefficient(matId));
}
void Lua::PhysSoftBody::GetMaterialLinearStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetMaterialLinearStiffnessCoefficient(matId));
}
void Lua::PhysSoftBody::GetMaterialVolumeStiffnessCoefficient(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t matId)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushNumber(l, hPhys->GetMaterialVolumeStiffnessCoefficient(matId));
}
void Lua::PhysSoftBody::AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot, bool bDisableCollision, float influence)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, localPivot, bDisableCollision, influence);
}
void Lua::PhysSoftBody::AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot, bool bDisableCollision)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, localPivot, bDisableCollision);
}
void Lua::PhysSoftBody::AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, const Vector3 &localPivot)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, localPivot);
}
void Lua::PhysSoftBody::AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, bool bDisableCollision, float influence)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, bDisableCollision, influence);
}
void Lua::PhysSoftBody::AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody, bool bDisableCollision)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody, bDisableCollision);
}
void Lua::PhysSoftBody::AppendAnchor(lua::State *l, pragma::physics::ISoftBody *hPhys, uint32_t nodeId, pragma::physics::IRigidBody *hRigidBody)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false || Lua::CheckHandle<pragma::physics::IRigidBody>(l, hRigidBody) == false)
		return;
	hPhys->AppendAnchor(nodeId, *hRigidBody);
}
void Lua::PhysSoftBody::GetNodeCount(lua::State *l, pragma::physics::ISoftBody *hPhys)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	PushInt(l, hPhys->GetNodeCount());
}
void Lua::PhysSoftBody::MeshVertexIndexToLocalVertexIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t meshVertexIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t localIndex = 0u;
	if(hPhys->MeshVertexIndexToLocalVertexIndex(meshVertexIndex, localIndex) == false)
		return;
	PushInt(l, localIndex);
}
void Lua::PhysSoftBody::LocalVertexIndexToMeshVertexIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t localIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t meshVertexIndex = 0u;
	if(hPhys->LocalVertexIndexToMeshVertexIndex(localIndex, meshVertexIndex) == false)
		return;
	PushInt(l, meshVertexIndex);
}
void Lua::PhysSoftBody::LocalVertexIndexToNodeIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t localVertexIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t nodeIndex = 0u;
	if(hPhys->LocalVertexIndexToNodeIndex(localVertexIndex, nodeIndex) == false)
		return;
	PushInt(l, nodeIndex);
}
void Lua::PhysSoftBody::NodeIndexToLocalVertexIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t nodeIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t localIndex = 0u;
	if(hPhys->NodeIndexToLocalVertexIndex(nodeIndex, localIndex) == false)
		return;
	PushInt(l, localIndex);
}

void Lua::PhysSoftBody::MeshVertexIndexToNodeIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t meshVertexIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t nodeIndex = 0u;
	if(hPhys->MeshVertexIndexToNodeIndex(meshVertexIndex, nodeIndex) == false)
		return;
	PushInt(l, nodeIndex);
}
void Lua::PhysSoftBody::NodeIndexToMeshVertexIndex(lua::State *l, pragma::physics::ISoftBody *&hPhys, uint32_t nodeIndex)
{
	if(Lua::CheckHandle<pragma::physics::ISoftBody>(l, hPhys) == false)
		return;
	uint16_t meshIndex = 0u;
	if(hPhys->NodeIndexToMeshVertexIndex(nodeIndex, meshIndex) == false)
		return;
	PushInt(l, meshIndex);
}

#ifndef __PHYSCOLLISIONOBJECT_H__
#define __PHYSCOLLISIONOBJECT_H__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <pragma/physics/physapi.h>
#include <mathutil/glmutil.h>
#include <mathutil/uquat.h>
#include "pragma/physics/phystransform.h"
#include "pragma/physics/physbase.h"
#include "pragma/lua/baseluaobj.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/networking/nwm_velocity_correction.hpp"
#include <vector>
#ifdef __linux__
#include "pragma/physics/physmotionstate.h"
#endif

class PhysCollisionObjectHandle;
class PhysEnv;
class PhysShape;
class PhysObj;
class PhysMotionState;
class DLLNETWORK PhysCollisionObject
	: public PhysBase,public LuaObj<PhysCollisionObjectHandle>
{
public:
	friend PhysEnv;
	friend PhysObj;
protected:
	PhysCollisionObject(PhysEnv *env,btCollisionObject *o,std::shared_ptr<PhysShape> &shape);
	void Initialize();

	btCollisionObject *m_collisionObject = nullptr;
	std::shared_ptr<PhysShape> m_shape;
	UInt32 m_boneId = 0u;
	Vector3 m_origin = {};
	Bool m_bSpawned = false;
	Bool m_bHasOrigin = false;
	Bool m_customSurfaceMaterial = false;
	int m_surfaceMaterial = 0u;
	bool m_bCcdEnabled = true;
	bool m_bUpdateAABB = false;
	std::pair<bool,CollisionMask> m_simulationEnabled = {true,CollisionMask::None};
	CollisionMask m_collisionFilterGroup = CollisionMask::Default;
	CollisionMask m_collisionFilterMask = CollisionMask::Default;
	virtual void AddWorldObject();
	virtual void RemoveWorldObject();
	virtual void InitializeHandle() override;
	void UpdateSurfaceMaterial();
	void UpdateCCD();
public:
	virtual ~PhysCollisionObject() override;
	virtual void Spawn();
	btCollisionObject *GetCollisionObject();
	int GetSurfaceMaterial() const;
	void SetSurfaceMaterial(int id);

	void SetCCDEnabled(bool b);
	virtual void GetAABB(Vector3 &min,Vector3 &max) const;
	void SetOrigin(const Vector3 &origin);
	Vector3 &GetOrigin();
	Bool HasOrigin() const;
	virtual Vector3 GetPos() const;
	virtual void SetPos(const Vector3 &pos);
	virtual Quat GetRotation() const;
	virtual void SetRotation(const Quat &rot);
	PhysTransform GetWorldTransform();
	virtual void SetWorldTransform(const PhysTransform &t);
	int GetCollisionFlags() const;
	void SetCollisionFlags(int flags);
	void Activate(bool forceActivation=false);
	int GetActivationState();
	void SetActivationState(int state);
	void SetCollisionShape(std::shared_ptr<PhysShape> &shape);
	std::shared_ptr<PhysShape> GetCollisionShape() const;
	void SetContactProcessingThreshold(float threshold);
	void SetTrigger(bool bTrigger);
	bool IsTrigger();
	virtual bool IsRigid() const;
	virtual bool IsGhost() const;
	virtual bool IsSoftBody() const;
	UInt32 GetBoneID() const;
	void SetBoneID(UInt32 id);

	void SetSimulationEnabled(bool b);
	void DisableSimulation();
	void EnableSimulation();
	bool IsSimulationEnabled() const;

	void SetCollisionFilterGroup(CollisionMask group);
	CollisionMask GetCollisionFilterGroup() const;
	void SetCollisionFilterMask(CollisionMask mask);
	CollisionMask GetCollisionFilterMask() const;

	virtual void PreSimulate();
	virtual void PostSimulate();

	void UpdateAABB();
	bool ShouldUpdateAABB() const;
	void ResetUpdateAABBFlag();

	void *userData = nullptr;
};

class DLLNETWORK PhysRigidBody
	: public PhysCollisionObject,
	public nwm::VelocityCorrection
{
public:
	friend PhysEnv;
protected:
	PhysRigidBody(PhysEnv *env,btRigidBody *body,float mass,std::shared_ptr<PhysShape> &shape,const Vector3 &localInertia);
	PhysRigidBody(PhysEnv *env,float mass,std::shared_ptr<PhysShape> &shape,const Vector3 &localInertia);

	struct KinematicData
	{
		Vector3 linearVelocity = {};
		Vector3 angularVelocity = {};
	};

	KinematicData m_kinematicData = {};

	btRigidBody *m_rigidBody = nullptr;
	float m_mass = 0.f;
	Vector3 m_inertia = {};
	std::unique_ptr<btMotionState> m_motionState = nullptr;
	virtual void AddWorldObject() override;
	virtual void RemoveWorldObject() override;
	virtual void InitializeHandle() override;
public:
	virtual ~PhysRigidBody() override;
	virtual void InitializeLuaObject(lua_State *lua) override;
	btRigidBody *GetRigidBody() const;
	virtual void SetPos(const Vector3 &pos) override;
	virtual Vector3 GetPos() const override;
	virtual Quat GetRotation() const override;
	virtual void SetRotation(const Quat &rot) override;
	void ApplyForce(const Vector3 &force);
	void ApplyForce(const Vector3 &force,const Vector3 &relPos);
	void ApplyImpulse(const Vector3 &impulse);
	void ApplyImpulse(const Vector3 &impulse,const Vector3 &relPos);
	void ApplyTorque(const Vector3 &torque);
	void ApplyTorqueImpulse(const Vector3 &torque);
	void ClearForces();
	Vector3 GetTotalForce();
	Vector3 GetTotalTorque();
	void SetMassProps(float mass,const Vector3 &inertia);
	float GetMass() const;
	void SetMass(float mass);
	Vector3 &GetInertia();
	Mat3 GetInvInertiaTensorWorld() const;
	void SetInertia(const Vector3 &inertia);
	int GetFlags();
	void SetFlags(int flags);
	Vector3 GetLinearVelocity();
	Vector3 GetAngularVelocity();
	void SetLinearVelocity(const Vector3 &vel);
	void SetAngularVelocity(const Vector3 &vel);
	void SetLinearFactor(const Vector3 &factor);
	void SetAngularFactor(const Vector3 &factor);
	Vector3 GetLinearFactor() const;
	Vector3 GetAngularFactor() const;
	virtual bool IsRigid() const override;
	void SetDamping(float linDamping,float angDamping);
	void SetLinearDamping(float damping);
	void SetAngularDamping(float damping);
	float GetLinearDamping() const;
	float GetAngularDamping() const;
	void SetLinearSleepingThreshold(float threshold);
	void SetAngularSleepingThreshold(float threshold);
	void SetSleepingThresholds(float linear,float angular);
	float GetLinearSleepingThreshold() const;
	float GetAngularSleepingThreshold() const;
	std::pair<float,float> GetSleepingThreshold() const;
	bool IsStatic() const;

	void SetKinematic(bool bKinematic);
	bool IsKinematic() const;

	virtual void PreSimulate() override;
	virtual void PostSimulate() override;
};

class DLLNETWORK PhysSoftBody
	: public PhysCollisionObject
{
public:
	friend PhysEnv;
protected:
	PhysSoftBody(PhysEnv *env,btSoftBody *o,std::shared_ptr<PhysShape> &shape,const std::vector<uint16_t> &meshVertIndicesToPhysIndices);

	btSoftBody *m_softBody = nullptr;
	Quat m_rotation = uquat::identity();
	float m_totalMass = 0.f;
	Vector3 m_linearVelocity = {};
	std::vector<uint16_t> m_meshVertexIndicesToLocalVertexIndices;
	std::vector<uint16_t> m_localVertexIndicesToMeshVertexIndices;

	std::vector<uint16_t> m_localVertexIndicesToNodeIndices;
	std::vector<uint16_t> m_nodeIndicesToLocalVertexIndices;
	std::weak_ptr<ModelSubMesh> m_subMesh = {};
	virtual void AddWorldObject() override;
	virtual void RemoveWorldObject() override;
	virtual void InitializeHandle() override;
	void UpdateTotalMass();
public:
	virtual void InitializeLuaObject(lua_State *lua) override;
	btSoftBody *GetSoftBody();
	virtual bool IsSoftBody() const override;
	virtual Vector3 GetPos() const override;
	virtual void SetPos(const Vector3 &pos) override;
	virtual void SetRotation(const Quat &rot) override;
	virtual Quat GetRotation() const override;
	virtual void SetWorldTransform(const PhysTransform &t) override;
	void UpdateLinearVelocity();

	const std::vector<uint16_t> &GetMeshVertexIndicesToLocalIndices() const;
	const std::vector<uint16_t> &GetLocalVertexIndicesToNodeIndices() const;
	const std::vector<uint16_t> &GetLocalVertexIndicesToMeshVertexIndices() const;
	const std::vector<uint16_t> &GetNodeIndicesToLocalVertexIndices() const;

	bool MeshVertexIndexToLocalVertexIndex(uint16_t meshVertexIndex,uint16_t &localIndex) const;
	bool LocalVertexIndexToMeshVertexIndex(uint16_t localIndex,uint16_t &meshVertexIndex) const;
	bool LocalVertexIndexToNodeIndex(uint16_t localVertexIndex,uint16_t &nodeIndex) const;
	bool NodeIndexToLocalVertexIndex(uint16_t nodeIndex,uint16_t &localVertexIndex) const;
	
	bool MeshVertexIndexToNodeIndex(uint16_t meshVertexIndex,uint16_t &nodeIndex) const;
	bool NodeIndexToMeshVertexIndex(uint16_t nodeIndex,uint16_t &meshVertexIndex) const;

	void SetSubMesh(const ModelSubMesh &subMesh,const std::vector<uint16_t> &meshVertexIndicesToLocalVertexIndices);
	ModelSubMesh *GetSubMesh() const;

	void AppendAnchor(uint32_t nodeId,PhysRigidBody &body,const Vector3 &localPivot,bool bDisableCollision=false,float influence=1.f);
	void AppendAnchor(uint32_t nodeId,PhysRigidBody &body,bool bDisableCollision=false,float influence=1.f);
	const btAlignedObjectArray<btSoftBody::Node> &GetNodes() const;
	uint32_t GetNodeCount() const;

	const Vector3 &GetLinearVelocity() const;

	virtual void GetAABB(Vector3 &min,Vector3 &max) const override;
	void AddAeroForceToNode(int32_t node,const Vector3 &force);
	void AddAeroForceToFace(int32_t face,const Vector3 &force);
	void AddForce(const Vector3 &force);
	void AddForce(uint32_t node,const Vector3 &force);
	void AddLinearVelocity(const Vector3 &vel);
	void AddLinearVelocity(uint32_t node,const Vector3 &vel);
	float GetFriction() const;
	float GetHitFraction() const;
	float GetRollingFriction() const;
	Vector3 GetAnisotropicFriction() const;
	void SetFriction(float friction);
	void SetHitFraction(float fraction);
	void SetRollingFriction(float friction);
	void SetAnisotropicFriction(const Vector3 &friction);
	float GetMass(int32_t node) const;
	float GetMass() const;
	float GetRestitution() const;
	float GetRestLengthScale() const;
	Vector3 GetWindVelocity() const;
	void SetMass(int32_t node,float mass);
	void SetMass(float mass);
	void SetRestitution(float rest);
	void SetRestLengthScale(float scale);
	void SetWindVelocity(const Vector3 &vel);
	void SetLinearVelocity(const Vector3 &vel);
	void SetVolumeDensity(float density);
	void SetVolumeMass(float mass);
	float GetVolume() const;
	void SetDensity(float density);

	void SetAnchorsHardness(float val);
	void SetRigidContactsHardness(float val);
	void SetDynamicFrictionCoefficient(float val);
	void SetDragCoefficient(float val);
	void SetDampingCoefficient(float val);
	void SetKineticContactsHardness(float val);
	void SetLiftCoefficient(float val);
	void SetPoseMatchingCoefficient(float val);
	void SetPressureCoefficient(float val);
	void SetSoftContactsHardness(float val);
	void SetSoftVsKineticHardness(float val);
	void SetSoftVsRigidImpulseSplitK(float val);
	void SetSoftVsRigidHardness(float val);
	void SetSoftVsRigidImpulseSplitR(float val);
	void SetSoftVsSoftHardness(float val);
	void SetSoftVsRigidImpulseSplitS(float val);
	void SetVolumeConversationCoefficient(float val);
	void SetVelocitiesCorrectionFactor(float val);

	float GetAnchorsHardness() const;
	float GetRigidContactsHardness() const;
	float GetDynamicFrictionCoefficient() const;
	float GetDragCoefficient() const;
	float GetDampingCoefficient() const;
	float GetKineticContactsHardness() const;
	float GetLiftCoefficient() const;
	float GetPoseMatchingCoefficient() const;
	float GetPressureCoefficient() const;
	float GetSoftContactsHardness() const;
	float GetSoftVsKineticHardness() const;
	float GetSoftVsRigidImpulseSplitK() const;
	float GetSoftVsRigidHardness() const;
	float GetSoftVsRigidImpulseSplitR() const;
	float GetSoftVsSoftHardness() const;
	float GetSoftVsRigidImpulseSplitS() const;
	float GetVolumeConversationCoefficient() const;
	float GetVelocitiesCorrectionFactor() const;

	void SetMaterialAngularStiffnessCoefficient(uint32_t matId,float val);
	void SetMaterialLinearStiffnessCoefficient(uint32_t matId,float val);
	void SetMaterialVolumeStiffnessCoefficient(uint32_t matId,float val);
	float GetMaterialAngularStiffnessCoefficient(uint32_t matId) const;
	float GetMaterialLinearStiffnessCoefficient(uint32_t matId) const;
	float GetMaterialVolumeStiffnessCoefficient(uint32_t matId) const;
};

class DLLNETWORK PhysGhostObject
	: public PhysCollisionObject
{
public:
	friend PhysEnv;
protected:
	PhysGhostObject(PhysEnv *env,btPairCachingGhostObject *o,std::shared_ptr<PhysShape> &shape);
	
	btPairCachingGhostObject *m_ghostObject = nullptr;
	virtual void AddWorldObject() override;
	virtual void RemoveWorldObject() override;
public:
	btPairCachingGhostObject *GetGhostObject();
	virtual bool IsGhost() const override;
};

DECLARE_BASE_HANDLE(DLLNETWORK,PhysCollisionObject,PhysCollisionObject);
DECLARE_DERIVED_HANDLE(DLLNETWORK,PhysCollisionObject,PhysCollisionObject,PhysRigidBody,PhysRigidBody);
DECLARE_DERIVED_HANDLE(DLLNETWORK,PhysCollisionObject,PhysCollisionObject,PhysSoftBody,PhysSoftBody);

#endif
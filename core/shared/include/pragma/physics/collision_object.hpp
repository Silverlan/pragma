#ifndef __PHYS_COLLISION_OBJECT_HPP__
#define __PHYS_COLLISION_OBJECT_HPP__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <mathutil/glmutil.h>
#include <mathutil/uquat.h>
#include "pragma/physics/transform.hpp"
#include "pragma/physics/base.hpp"
#include "pragma/lua/baseluaobj.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/networking/nwm_velocity_correction.hpp"
#include <vector>
#ifdef __linux__
#include "pragma/physics/physmotionstate.h"
#endif

class ModelSubMesh;
namespace pragma::physics
{
	class IShape;
	class IRigidBody;
	class ISoftBody;
	class IGhostObject;
	class DLLNETWORK ICollisionObject
		: public IBase,public IWorldObject
	{
	public:
		enum class ActivationState : uint32_t
		{
			Active = 0,
			AlwaysActive,
			AlwaysInactive,
			Asleep,
			WaitForDeactivation,

			Count
		};

		virtual void Spawn();
		virtual void OnRemove() override;
		int GetSurfaceMaterial() const;
		void SetSurfaceMaterial(int id);
		void SetOrigin(const Vector3 &origin);
		const Vector3 &GetOrigin() const;
		Bool HasOrigin() const;

		UInt32 GetBoneID() const;
		void SetBoneID(UInt32 id);
		void SetCollisionShape(pragma::physics::IShape *shape);
		const pragma::physics::IShape *GetCollisionShape() const;
		pragma::physics::IShape *GetCollisionShape();
		
		virtual bool IsRigid() const;
		virtual bool IsGhost() const;
		virtual bool IsSoftBody() const;

		virtual void InitializeLuaObject(lua_State *lua) override;

		virtual void SetActivationState(ActivationState state)=0;
		virtual ActivationState GetActivationState() const=0;

		virtual void SetContactProcessingThreshold(float threshold)=0;
		virtual bool IsStatic() const=0;
		virtual void SetStatic(bool b)=0;

		virtual void SetCCDEnabled(bool b)=0;
		virtual void GetAABB(Vector3 &min,Vector3 &max) const=0;
		virtual Vector3 GetPos() const=0;
		virtual void SetPos(const Vector3 &pos)=0;
		virtual Quat GetRotation() const=0;
		virtual void SetRotation(const Quat &rot)=0;
		virtual Transform GetWorldTransform()=0;
		virtual void SetWorldTransform(const Transform &t)=0;
		virtual void WakeUp(bool forceActivation=false)=0;
		virtual void PutToSleep()=0;
		virtual bool IsAsleep() const=0;

		virtual bool IsTrigger()=0;

		virtual void SetSimulationEnabled(bool b)=0;
		void DisableSimulation();
		void EnableSimulation();
		virtual bool IsSimulationEnabled() const=0;
		virtual void SetCollisionsEnabled(bool enabled)=0;

		void SetCollisionFilterGroup(CollisionMask group);
		CollisionMask GetCollisionFilterGroup() const;
		void SetCollisionFilterMask(CollisionMask mask);
		CollisionMask GetCollisionFilterMask() const;

		virtual void PreSimulate();
		virtual void PostSimulate();

		void UpdateAABB();
		bool ShouldUpdateAABB() const;
		void ResetUpdateAABBFlag();

		virtual IRigidBody *GetRigidBody();
		const IRigidBody *GetRigidBody() const;
		virtual ISoftBody *GetSoftBody();
		const ISoftBody *GetSoftBody() const;
		virtual IGhostObject *GetGhostObject();
		const IGhostObject *GetGhostObject() const;

		virtual void InitializeLuaHandle(lua_State *l,const util::TWeakSharedHandle<IBase> &handle) override;
	protected:
		ICollisionObject(pragma::physics::IEnvironment &env,pragma::physics::IShape &shape);
		virtual void ApplyCollisionShape(pragma::physics::IShape *optShape)=0;
		virtual void DoSetCollisionFilterGroup(CollisionMask group)=0;
		virtual void DoSetCollisionFilterMask(CollisionMask mask)=0;

		std::shared_ptr<pragma::physics::IShape> m_shape;
		UInt32 m_boneId = 0u;
		Vector3 m_origin = {};
		Bool m_bHasOrigin = false;
		Bool m_customSurfaceMaterial = false;
		int m_surfaceMaterial = 0u;
		bool m_bCcdEnabled = true;
		bool m_bUpdateAABB = false;
		std::pair<bool,CollisionMask> m_simulationEnabled = {true,CollisionMask::None};
		CollisionMask m_collisionFilterGroup = CollisionMask::Default;
		CollisionMask m_collisionFilterMask = CollisionMask::Default;

		void UpdateSurfaceMaterial();
	};

	class DLLNETWORK IGhostObject
		: virtual public ICollisionObject
	{
	public:
		virtual bool IsGhost() const override;
		virtual IGhostObject *GetGhostObject() override;
		virtual void InitializeLuaObject(lua_State *lua) override;
	protected:
		using ICollisionObject::ICollisionObject;
	};

	class DLLNETWORK IRigidBody
		: virtual public ICollisionObject
	{
	public:
		virtual void InitializeLuaObject(lua_State *lua) override;
		virtual bool IsRigid() const override;
		virtual IRigidBody *GetRigidBody() override;
		virtual void ApplyForce(const Vector3 &force)=0;
		virtual void ApplyForce(const Vector3 &force,const Vector3 &relPos)=0;
		virtual void ApplyImpulse(const Vector3 &impulse)=0;
		virtual void ApplyImpulse(const Vector3 &impulse,const Vector3 &relPos)=0;
		virtual void ApplyTorque(const Vector3 &torque)=0;
		virtual void ApplyTorqueImpulse(const Vector3 &torque)=0;
		virtual void ClearForces()=0;
		virtual Vector3 GetTotalForce() const=0;
		virtual Vector3 GetTotalTorque() const=0;
		virtual void SetMassProps(float mass,const Vector3 &inertia)=0;
		virtual float GetMass() const=0;
		virtual void SetMass(float mass)=0;
		virtual Vector3 &GetInertia()=0;
		virtual Mat3 GetInvInertiaTensorWorld() const=0;
		virtual void SetInertia(const Vector3 &inertia)=0;
		virtual Vector3 GetLinearVelocity() const=0;
		virtual Vector3 GetAngularVelocity() const=0;
		virtual void SetLinearVelocity(const Vector3 &vel)=0;
		virtual void SetAngularVelocity(const Vector3 &vel)=0;
		virtual void SetLinearFactor(const Vector3 &factor)=0;
		virtual void SetAngularFactor(const Vector3 &factor)=0;
		virtual Vector3 GetLinearFactor() const=0;
		virtual Vector3 GetAngularFactor() const=0;
		void SetDamping(float linDamping,float angDamping);
		virtual void SetLinearDamping(float damping)=0;
		virtual void SetAngularDamping(float damping)=0;
		virtual float GetLinearDamping() const=0;
		virtual float GetAngularDamping() const=0;
		virtual void SetLinearSleepingThreshold(float threshold)=0;
		virtual void SetAngularSleepingThreshold(float threshold)=0;
		void SetSleepingThresholds(float linear,float angular);
		virtual float GetLinearSleepingThreshold() const=0;
		virtual float GetAngularSleepingThreshold() const=0;
		std::pair<float,float> GetSleepingThreshold() const;

		virtual void SetKinematic(bool bKinematic)=0;
		virtual bool IsKinematic() const=0;
	protected:
		IRigidBody(IEnvironment &env,float mass,pragma::physics::IShape &shape,const Vector3 &localInertia);
	};

	class DLLNETWORK ISoftBody
		: virtual public ICollisionObject
	{
	public:
		virtual void InitializeLuaObject(lua_State *lua) override;
		virtual bool IsSoftBody() const override;
		virtual ISoftBody *GetSoftBody() override;
		virtual void AddVelocity(const Vector3 &vel)=0;

		virtual const std::vector<uint16_t> &GetMeshVertexIndicesToLocalIndices() const=0;
		virtual const std::vector<uint16_t> &GetLocalVertexIndicesToNodeIndices() const=0;
		virtual const std::vector<uint16_t> &GetLocalVertexIndicesToMeshVertexIndices() const=0;
		virtual const std::vector<uint16_t> &GetNodeIndicesToLocalVertexIndices() const=0;

		virtual bool MeshVertexIndexToLocalVertexIndex(uint16_t meshVertexIndex,uint16_t &localIndex) const=0;
		virtual bool LocalVertexIndexToMeshVertexIndex(uint16_t localIndex,uint16_t &meshVertexIndex) const=0;
		virtual bool LocalVertexIndexToNodeIndex(uint16_t localVertexIndex,uint16_t &nodeIndex) const=0;
		virtual bool NodeIndexToLocalVertexIndex(uint16_t nodeIndex,uint16_t &localVertexIndex) const=0;

		virtual bool MeshVertexIndexToNodeIndex(uint16_t meshVertexIndex,uint16_t &nodeIndex) const=0;
		virtual bool NodeIndexToMeshVertexIndex(uint16_t nodeIndex,uint16_t &meshVertexIndex) const=0;

		virtual void SetSubMesh(const ModelSubMesh &subMesh,const std::vector<uint16_t> &meshVertexIndicesToLocalVertexIndices)=0;
		ModelSubMesh *GetSubMesh() const;

		virtual void UpdateLinearVelocity()=0;

		virtual void AppendAnchor(uint32_t nodeId,IRigidBody &body,const Vector3 &localPivot,bool bDisableCollision=false,float influence=1.f)=0;
		virtual void AppendAnchor(uint32_t nodeId,IRigidBody &body,bool bDisableCollision=false,float influence=1.f)=0;
		virtual uint32_t GetNodeCount() const=0;

		virtual const Vector3 &GetLinearVelocity() const=0;

		virtual void AddAeroForceToNode(int32_t node,const Vector3 &force)=0;
		virtual void AddAeroForceToFace(int32_t face,const Vector3 &force)=0;
		virtual void AddForce(const Vector3 &force)=0;
		virtual void AddForce(uint32_t node,const Vector3 &force)=0;
		virtual void AddLinearVelocity(const Vector3 &vel)=0;
		virtual void AddLinearVelocity(uint32_t node,const Vector3 &vel)=0;
		virtual float GetFriction() const=0;
		virtual float GetHitFraction() const=0;
		virtual float GetRollingFriction() const=0;
		virtual Vector3 GetAnisotropicFriction() const=0;
		virtual void SetFriction(float friction)=0;
		virtual void SetHitFraction(float fraction)=0;
		virtual void SetRollingFriction(float friction)=0;
		virtual void SetAnisotropicFriction(const Vector3 &friction)=0;
		virtual float GetMass(int32_t node) const=0;
		virtual float GetMass() const=0;
		virtual float GetRestitution() const=0;
		virtual float GetRestLengthScale() const=0;
		virtual Vector3 GetWindVelocity() const=0;
		virtual void SetMass(int32_t node,float mass)=0;
		virtual void SetMass(float mass)=0;
		virtual void SetRestitution(float rest)=0;
		virtual void SetRestLengthScale(float scale)=0;
		virtual void SetWindVelocity(const Vector3 &vel)=0;
		virtual void SetLinearVelocity(const Vector3 &vel)=0;
		virtual void SetVolumeDensity(float density)=0;
		virtual void SetVolumeMass(float mass)=0;
		virtual float GetVolume() const=0;
		virtual void SetDensity(float density)=0;

		virtual void SetAnchorsHardness(float val)=0;
		virtual void SetRigidContactsHardness(float val)=0;
		virtual void SetDynamicFrictionCoefficient(float val)=0;
		virtual void SetDragCoefficient(float val)=0;
		virtual void SetDampingCoefficient(float val)=0;
		virtual void SetKineticContactsHardness(float val)=0;
		virtual void SetLiftCoefficient(float val)=0;
		virtual void SetPoseMatchingCoefficient(float val)=0;
		virtual void SetPressureCoefficient(float val)=0;
		virtual void SetSoftContactsHardness(float val)=0;
		virtual void SetSoftVsKineticHardness(float val)=0;
		virtual void SetSoftVsRigidImpulseSplitK(float val)=0;
		virtual void SetSoftVsRigidHardness(float val)=0;
		virtual void SetSoftVsRigidImpulseSplitR(float val)=0;
		virtual void SetSoftVsSoftHardness(float val)=0;
		virtual void SetSoftVsRigidImpulseSplitS(float val)=0;
		virtual void SetVolumeConversationCoefficient(float val)=0;
		virtual void SetVelocitiesCorrectionFactor(float val)=0;

		virtual float GetAnchorsHardness() const=0;
		virtual float GetRigidContactsHardness() const=0;
		virtual float GetDynamicFrictionCoefficient() const=0;
		virtual float GetDragCoefficient() const=0;
		virtual float GetDampingCoefficient() const=0;
		virtual float GetKineticContactsHardness() const=0;
		virtual float GetLiftCoefficient() const=0;
		virtual float GetPoseMatchingCoefficient() const=0;
		virtual float GetPressureCoefficient() const=0;
		virtual float GetSoftContactsHardness() const=0;
		virtual float GetSoftVsKineticHardness() const=0;
		virtual float GetSoftVsRigidImpulseSplitK() const=0;
		virtual float GetSoftVsRigidHardness() const=0;
		virtual float GetSoftVsRigidImpulseSplitR() const=0;
		virtual float GetSoftVsSoftHardness() const=0;
		virtual float GetSoftVsRigidImpulseSplitS() const=0;
		virtual float GetVolumeConversationCoefficient() const=0;
		virtual float GetVelocitiesCorrectionFactor() const=0;

		virtual void SetMaterialAngularStiffnessCoefficient(uint32_t matId,float val)=0;
		virtual void SetMaterialLinearStiffnessCoefficient(uint32_t matId,float val)=0;
		virtual void SetMaterialVolumeStiffnessCoefficient(uint32_t matId,float val)=0;
		virtual float GetMaterialAngularStiffnessCoefficient(uint32_t matId) const=0;
		virtual float GetMaterialLinearStiffnessCoefficient(uint32_t matId) const=0;
		virtual float GetMaterialVolumeStiffnessCoefficient(uint32_t matId) const=0;
	protected:
		ISoftBody(IEnvironment &env,pragma::physics::IShape &shape,const std::vector<uint16_t> &meshVertIndicesToPhysIndices);
		std::weak_ptr<ModelSubMesh> m_subMesh = {};
	};
};

#endif
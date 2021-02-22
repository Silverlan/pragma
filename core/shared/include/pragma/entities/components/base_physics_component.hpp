/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __BASE_PHYSICS_COMPONENT_HPP__
#define __BASE_PHYSICS_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/physics/physicstypes.h"
#include <sharedutils/util_shared_handle.hpp>

class Frame;
struct Bone;
enum class MOVETYPE : int;
enum class COLLISIONTYPE : int;
namespace pragma
{
	namespace physics
	{
		class IConvexShape; class IRigidBody; class IConstraint; class IShape; class PhysObjCreateInfo;

		struct DLLNETWORK PhysObjCreateInfo
		{
			using BoneId = int32_t;
			using MeshIndex = uint32_t;
			struct DLLNETWORK ShapeInfo
			{
				std::weak_ptr<pragma::physics::IShape> shape = {};
				umath::Transform localPose = {};
			};
			// Returns the number of shapes that have been added
			uint32_t AddShape(pragma::physics::IShape &shape,const umath::Transform &localPose={},BoneId boneId=-1);
			void SetModelMeshBoneMapping(MeshIndex modelMeshIndex,BoneId boneIndex);
			void SetModel(Model &model);
			Model *GetModel() const;
			const std::unordered_map<BoneId,std::vector<ShapeInfo>> &GetShapes() const;
			const std::unordered_map<MeshIndex,BoneId> &GetModelMeshBoneMappings() const;
		private:
			std::unordered_map<BoneId,std::vector<ShapeInfo>> m_shapes = {};
			std::unordered_map<MeshIndex,BoneId> m_modelMeshIndexToShapeIndex = {};
			util::WeakHandle<Model> m_model = {};
		};
	};
	struct DLLNETWORK CEPhysicsUpdateData
		: public ComponentEvent
	{
		CEPhysicsUpdateData(double deltaTime);
		virtual void PushArguments(lua_State *l) override;
		double deltaTime;
	};
	struct DLLNETWORK CEHandleRaycast
		: public ComponentEvent
	{
		CEHandleRaycast(CollisionMask rayCollisionGroup,CollisionMask rayCollisionMask);
		virtual void PushArguments(lua_State *l) override;
		CollisionMask rayCollisionGroup;
		CollisionMask rayCollisionMask;
		bool hit = false;
	};
	class DLLNETWORK BasePhysicsComponent
		: public BaseEntityComponent
	{
	protected:
		struct DLLNETWORK CollisionInfo
		{
			CollisionInfo();
			CollisionInfo(BaseEntity *ent,Bool shouldCollide);
			EntityHandle entity;
			Bool shouldCollide;
		};
		struct DLLNETWORK PhysJoint
		{
			PhysJoint(unsigned int source,unsigned int target);
			unsigned int source; // Bone ID
			unsigned int target; // Bone ID
			util::TSharedHandle<pragma::physics::IConstraint> constraint;
		};
	public:
		enum class StateFlags : uint32_t
		{
			None = 0u,
			Ragdoll = 1u,
			Kinematic = Ragdoll<<1u,
			CollisionsEnabled = Kinematic<<1u,
			ApplyingLinearVelocity = CollisionsEnabled<<1u,
			ApplyingAngularVelocity = ApplyingLinearVelocity<<1u,
			ApplyingPhysicsPosition = ApplyingAngularVelocity<<1u,
			ApplyingPhysicsRotation = ApplyingPhysicsPosition<<1u,
			SleepReportEnabled = ApplyingPhysicsRotation<<1u,
			ForcePhysicsAwakeCallbacksEnabled = SleepReportEnabled<<1u,
			SimulationEnabled = ForcePhysicsAwakeCallbacksEnabled<<1u
		};

		enum class PhysFlags : uint32_t
		{
			None = 0u,
			Dynamic = 1u
		};

		static ComponentEventId EVENT_ON_PHYSICS_INITIALIZED;
		static ComponentEventId EVENT_ON_PHYSICS_DESTROYED;
		static ComponentEventId EVENT_ON_PHYSICS_UPDATED;
		static ComponentEventId EVENT_ON_DYNAMIC_PHYSICS_UPDATED;
		static ComponentEventId EVENT_ON_PRE_PHYSICS_SIMULATE;
		static ComponentEventId EVENT_ON_POST_PHYSICS_SIMULATE;
		static ComponentEventId EVENT_ON_SLEEP;
		static ComponentEventId EVENT_ON_WAKE;
		static ComponentEventId EVENT_HANDLE_RAYCAST;
		static ComponentEventId EVENT_INITIALIZE_PHYSICS;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		virtual void Initialize() override;
		virtual void OnRemove() override;

		const std::vector<PhysJoint> &GetJoints() const;
		std::vector<PhysJoint> &GetJoints();

		bool IsRagdoll() const;
		Vector3 GetCenter() const;

		float GetAABBDistance(const Vector3 &p) const;
		float GetAABBDistance(const BaseEntity &ent) const;

		virtual bool ShouldCollide(PhysObj *physThis,pragma::physics::ICollisionObject *colThis,BaseEntity *entOther,PhysObj *physOther,pragma::physics::ICollisionObject *colOther,bool valDefault) const;
		void ResetCollisions(BaseEntity *ent);
		void DisableCollisions(BaseEntity *ent);
		void EnableCollisions(BaseEntity *ent);
		void SetCollisionsEnabled(BaseEntity *ent,bool b);
		void UpdateCCD();
		void SetCollisionCallbacksEnabled(bool b);
		bool GetCollisionCallbacksEnabled() const;
		void SetCollisionContactReportEnabled(bool b);
		bool GetCollisionContactReportEnabled() const;
#if PHYS_KEEP_SIMULATION_TRANSFORM != 0
		Vector3 GetPhysicsSimulationOffset();
		Quat GetPhysicsSimulationRotation();
#endif
		virtual void PrePhysicsSimulate();
		virtual bool PostPhysicsSimulate();
		virtual void SetKinematic(bool b);
		bool IsKinematic() const;
		virtual void OnPhysicsWake(PhysObj *phys);
		virtual void OnPhysicsSleep(PhysObj *phys);
		bool IsOnGround() const;
		bool IsGroundWalkable() const;

		void SetForcePhysicsAwakeCallbacksEnabled(bool enabled,bool apply=true);
		bool AreForcePhysicsAwakeCallbacksEnabled() const;

		BaseEntity *GetGroundEntity() const;
		PhysObj *GetPhysicsObject() const;
		pragma::physics::ICollisionObject *GetCollisionObject(UInt32 boneId) const;
		virtual PhysObj *InitializePhysics(PHYSICSTYPE type,PhysFlags flags=PhysFlags::None);
		PhysObj *InitializePhysics(pragma::physics::IConvexShape &shape,PhysFlags flags=PhysFlags::None);
		virtual void DestroyPhysicsObject();
		PHYSICSTYPE GetPhysicsType() const;
		void DropToFloor();
		bool IsTrigger() const;
		virtual void SetCollisionFilter(CollisionMask filterGroup,CollisionMask filterMask);
		void AddCollisionFilter(CollisionMask filter);
		void RemoveCollisionFilter(CollisionMask filter);
		// Sets both the filterGroup AND filterMask to the specified value
		void SetCollisionFilter(CollisionMask filterGroup);
		void SetCollisionFilterMask(CollisionMask filterMask);
		void SetCollisionFilterGroup(CollisionMask filterGroup);
		CollisionMask GetCollisionFilter() const;
		CollisionMask GetCollisionFilterMask() const;
		void GetCollisionFilter(CollisionMask *filterGroup,CollisionMask *filterMask) const;
		virtual void PhysicsUpdate(double tDelta);
		void RayCast(const Vector3 &dir,float distance) const;
		void Sweep(const Vector3 &dir,float distance) const;
		// Is called after the world physics have been simulated
		virtual void UpdatePhysicsData();
		// Return false to discard result
		virtual bool RayResultCallback(CollisionMask rayCollisionGroup,CollisionMask rayCollisionMask);
		bool IsRayResultCallbackEnabled() const;
		void SetRayResultCallbackEnabled(bool b);

		MOVETYPE GetMoveType() const;
		virtual void SetMoveType(MOVETYPE movetype);
		COLLISIONTYPE GetCollisionType() const;
		virtual void SetCollisionType(COLLISIONTYPE collisiontype);

		std::vector<PhysJoint> &GetPhysConstraints();

		void ApplyForce(const Vector3 &force);
		void ApplyForce(const Vector3 &force,const Vector3 &relPos);
		void ApplyImpulse(const Vector3 &impulse);
		void ApplyImpulse(const Vector3 &impulse,const Vector3 &relPos);
		void ApplyTorque(const Vector3 &torque);
		void ApplyTorqueImpulse(const Vector3 &torque);
		float GetMass() const;
		virtual void SetCollisionsEnabled(bool b);
		bool GetCollisionsEnabled() const;
		virtual void SetSimulationEnabled(bool b);
		bool GetSimulationEnabled() const;

		void GetCollisionBounds(Vector3 *min,Vector3 *max) const;
		float GetCollisionRadius(Vector3 *center=nullptr) const;
		virtual void SetCollisionBounds(const Vector3 &min,const Vector3 &max);
		Vector3 GetCollisionExtents() const;
		Vector3 GetCollisionCenter() const;
		void GetRotatedCollisionBounds(Vector3 *min,Vector3 *max) const;
		StateFlags GetStateFlags() const;

		Vector3 GetOrigin() const;
		const Vector3 &GetLocalOrigin() const;

		void OriginToWorld(Vector3 *origin) const;
		void OriginToWorld(Vector3 *origin,Quat *rot) const;

		void WorldToOrigin(Vector3 *origin) const;
		void WorldToOrigin(Vector3 *origin,Quat *rot) const;

		virtual void Save(DataStream &ds) override;
		virtual void Load(DataStream &ds,uint32_t version) override;

		void SetSleepReportEnabled(bool reportEnabled);
		bool IsSleepReportEnabled() const;
		virtual void OnWake();
		virtual void OnSleep();

		// Should only be called from within an EVENT_INITIALIZE_PHYSICS event!
		util::WeakHandle<PhysObj> InitializePhysics(const physics::PhysObjCreateInfo &physObjCreateInfo,PhysFlags flags,int32_t rootMeshBoneId=-1);
	protected:
		BasePhysicsComponent(BaseEntity &ent);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;

		// Mass used for initialization; Not necessarily the same as the mass of the actual physics object
		virtual float GetPhysicsMass() const;
		void UpdateBoneCollisionObject(UInt32 boneId,Bool updatePos=true,Bool updateRot=false);
		
		pragma::NetEventId m_netEvSetCollisionsEnabled = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetSimEnabled = pragma::INVALID_NET_EVENT;

		bool m_bRayResultCallbackEnabled = false;
		PHYSICSTYPE m_physicsType = PHYSICSTYPE::NONE;
		std::shared_ptr<PhysObj> m_physObject = nullptr;
		std::vector<PhysJoint> m_joints;
		std::vector<CollisionInfo> m_customCollisions;
		std::vector<CollisionInfo>::iterator FindCollisionInfo(BaseEntity *ent);
		util::TSharedHandle<pragma::physics::IRigidBody> CreateRigidBody(pragma::physics::IShape &shape,bool dynamic,const umath::Transform &localPose={});
		util::WeakHandle<PhysObj> InitializeSoftBodyPhysics();
		util::WeakHandle<PhysObj> InitializeModelPhysics(PhysFlags flags=PhysFlags::Dynamic);
		util::WeakHandle<PhysObj> InitializeBoxControllerPhysics();
		util::WeakHandle<PhysObj> InitializeCapsuleControllerPhysics();
		virtual void OnPhysicsInitialized();
		virtual void OnPhysicsDestroyed();
		CollisionMask m_collisionFilterGroup = CollisionMask::Default;
		CollisionMask m_collisionFilterMask = CollisionMask::Default;
		bool m_bColCallbacksEnabled = false;
		bool m_bColContactReportEnabled = false;
		virtual void InitializePhysObj();
		void UpdatePhysicsBone(Frame &reference,const std::shared_ptr<Bone> &bone,Quat &invRot,const Vector3 *mvOffset=nullptr);
		void PostPhysicsSimulate(Frame &reference,std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones,Vector3 &moveOffset,Quat &invRot,UInt32 physRootBoneId);
		// Updates the entity's bones to match the transforms of the collision objects
		void UpdateRagdollPose();
		
		StateFlags m_stateFlags = StateFlags::CollisionsEnabled;
		MOVETYPE m_moveType = {};
		COLLISIONTYPE m_collisionType = {};
		float m_colRadius = 0.f;
		Vector3 m_colMin = {};
		Vector3 m_colMax = {};
	private:
		void ClearAwakeStatus();
	};
	struct DLLNETWORK CEInitializePhysics
		: public ComponentEvent
	{
		CEInitializePhysics(PHYSICSTYPE type,BasePhysicsComponent::PhysFlags flags);
		virtual void PushArguments(lua_State *l) override;
		PHYSICSTYPE physicsType;
		BasePhysicsComponent::PhysFlags flags;
	};
	struct DLLNETWORK CEPostPhysicsSimulate
		: public ComponentEvent
	{
		CEPostPhysicsSimulate();
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		bool keepAwake = true;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BasePhysicsComponent::StateFlags);
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BasePhysicsComponent::PhysFlags)

#endif

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.base_physics;

export import :entities.components.base;
export import :physics;
export import :types;

export {
	class Frame;
	namespace pragma {
		namespace physics {
			struct DLLNETWORK PhysObjCreateInfo {
				using BoneId = int32_t;
				using MeshIndex = uint32_t;
				struct DLLNETWORK ShapeInfo {
					std::weak_ptr<pragma::physics::IShape> shape = {};
					umath::Transform localPose = {};
				};
				// Returns the number of shapes that have been added
				uint32_t AddShape(pragma::physics::IShape &shape, const umath::Transform &localPose = {}, BoneId boneId = -1);
				void SetModelMeshBoneMapping(MeshIndex modelMeshIndex, BoneId boneIndex);
				void SetModel(pragma::Model &model);
				pragma::Model *GetModel() const;
				const std::unordered_map<BoneId, std::vector<ShapeInfo>> &GetShapes() const;
				const std::unordered_map<MeshIndex, BoneId> &GetModelMeshBoneMappings() const;
			private:
				std::unordered_map<BoneId, std::vector<ShapeInfo>> m_shapes = {};
				std::unordered_map<MeshIndex, BoneId> m_modelMeshIndexToShapeIndex = {};
				util::WeakHandle<pragma::Model> m_model = {};
			};
		};
		struct DLLNETWORK CEPhysicsUpdateData : public ComponentEvent {
			CEPhysicsUpdateData(double deltaTime);
			virtual void PushArguments(lua_State *l) override;
			double deltaTime;
		};
		struct DLLNETWORK CEHandleRaycast : public ComponentEvent {
			CEHandleRaycast(pragma::physics::CollisionMask rayCollisionGroup, pragma::physics::CollisionMask rayCollisionMask);
			virtual void PushArguments(lua_State *l) override;
			pragma::physics::CollisionMask rayCollisionGroup;
			pragma::physics::CollisionMask rayCollisionMask;
			bool hit = false;
		};
		class DLLNETWORK BasePhysicsComponent : public BaseEntityComponent {
		protected:
			struct DLLNETWORK CollisionInfo {
				CollisionInfo();
				CollisionInfo(pragma::ecs::BaseEntity *ent, Bool shouldCollide);
				EntityHandle entity;
				Bool shouldCollide;
			};
			struct DLLNETWORK PhysJoint {
				PhysJoint(unsigned int source, unsigned int target);
				unsigned int source; // Bone ID
				unsigned int target; // Bone ID
				util::TSharedHandle<pragma::physics::IConstraint> constraint;
			};
		public:
			enum class StateFlags : uint32_t {
				None = 0u,
				Ragdoll = 1u,
				Kinematic = Ragdoll << 1u,
				CollisionsEnabled = Kinematic << 1u,
				ApplyingLinearVelocity = CollisionsEnabled << 1u,
				ApplyingAngularVelocity = ApplyingLinearVelocity << 1u,
				ApplyingPhysicsPosition = ApplyingAngularVelocity << 1u,
				ApplyingPhysicsRotation = ApplyingPhysicsPosition << 1u,
				SleepReportEnabled = ApplyingPhysicsRotation << 1u,
				ForcePhysicsAwakeCallbacksEnabled = SleepReportEnabled << 1u,
				SimulationEnabled = ForcePhysicsAwakeCallbacksEnabled << 1u
			};

			enum class PhysFlags : uint32_t { None = 0u, Dynamic = 1u };

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
			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

			virtual void Initialize() override;
			virtual void OnRemove() override;

			const std::vector<PhysJoint> &GetJoints() const;
			std::vector<PhysJoint> &GetJoints();

			bool IsRagdoll() const;
			Vector3 GetCenter() const;

			float GetAABBDistance(const Vector3 &p) const;
			float GetAABBDistance(const pragma::ecs::BaseEntity &ent) const;

			virtual bool ShouldCollide(pragma::physics::PhysObj *physThis, pragma::physics::ICollisionObject *colThis, pragma::ecs::BaseEntity *entOther, pragma::physics::PhysObj *physOther, pragma::physics::ICollisionObject *colOther, bool valDefault) const;
			void ResetCollisions(pragma::ecs::BaseEntity *ent);
			void DisableCollisions(pragma::ecs::BaseEntity *ent);
			void EnableCollisions(pragma::ecs::BaseEntity *ent);
			void SetCollisionsEnabled(pragma::ecs::BaseEntity *ent, bool b);
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
			virtual void OnPhysicsWake(pragma::physics::PhysObj *phys);
			virtual void OnPhysicsSleep(pragma::physics::PhysObj *phys);
			bool IsOnGround() const;
			bool IsGroundWalkable() const;

			void SetForcePhysicsAwakeCallbacksEnabled(bool enabled, bool apply = true, std::optional<bool> isAwakeOverride = {});
			bool AreForcePhysicsAwakeCallbacksEnabled() const;

			pragma::ecs::BaseEntity *GetGroundEntity() const;
			pragma::physics::PhysObj *GetPhysicsObject() const;
			pragma::physics::ICollisionObject *GetCollisionObject(UInt32 boneId) const;
			virtual pragma::physics::PhysObj *InitializePhysics(pragma::physics::PHYSICSTYPE type, PhysFlags flags = PhysFlags::None);
			pragma::physics::PhysObj *InitializePhysics(pragma::physics::IConvexShape &shape, PhysFlags flags = PhysFlags::None);
			virtual void DestroyPhysicsObject();
			pragma::physics::PHYSICSTYPE GetPhysicsType() const;
			void DropToFloor();
			bool IsTrigger() const;
			virtual void SetCollisionFilter(pragma::physics::CollisionMask filterGroup, pragma::physics::CollisionMask filterMask);
			void AddCollisionFilter(pragma::physics::CollisionMask filter);
			void RemoveCollisionFilter(pragma::physics::CollisionMask filter);
			// Sets both the filterGroup AND filterMask to the specified value
			void SetCollisionFilter(pragma::physics::CollisionMask filterGroup);
			void SetCollisionFilterMask(pragma::physics::CollisionMask filterMask);
			void SetCollisionFilterGroup(pragma::physics::CollisionMask filterGroup);
			pragma::physics::CollisionMask GetCollisionFilter() const;
			pragma::physics::CollisionMask GetCollisionFilterMask() const;
			void GetCollisionFilter(pragma::physics::CollisionMask *filterGroup, pragma::physics::CollisionMask *filterMask) const;
			virtual void PhysicsUpdate(double tDelta);
			void RayCast(const Vector3 &dir, float distance) const;
			void Sweep(const Vector3 &dir, float distance) const;
			// Is called after the world physics have been simulated
			virtual void UpdatePhysicsData();
			// Return false to discard result
			virtual bool RayResultCallback(pragma::physics::CollisionMask rayCollisionGroup, pragma::physics::CollisionMask rayCollisionMask);
			bool IsRayResultCallbackEnabled() const;
			void SetRayResultCallbackEnabled(bool b);

			pragma::physics::MOVETYPE GetMoveType() const;
			virtual void SetMoveType(pragma::physics::MOVETYPE movetype);
			pragma::physics::COLLISIONTYPE GetCollisionType() const;
			virtual void SetCollisionType(pragma::physics::COLLISIONTYPE collisiontype);

			std::vector<PhysJoint> &GetPhysConstraints();

			void ApplyForce(const Vector3 &force);
			void ApplyForce(const Vector3 &force, const Vector3 &relPos);
			void ApplyImpulse(const Vector3 &impulse);
			void ApplyImpulse(const Vector3 &impulse, const Vector3 &relPos);
			void ApplyTorque(const Vector3 &torque);
			void ApplyTorqueImpulse(const Vector3 &torque);
			float GetMass() const;
			virtual void SetCollisionsEnabled(bool b);
			bool GetCollisionsEnabled() const;
			virtual void SetSimulationEnabled(bool b);
			bool GetSimulationEnabled() const;

			void GetCollisionBounds(Vector3 *min, Vector3 *max) const;
			float GetCollisionRadius(Vector3 *center = nullptr) const;
			virtual void SetCollisionBounds(const Vector3 &min, const Vector3 &max);
			Vector3 GetCollisionExtents() const;
			Vector3 GetCollisionCenter() const;
			void GetRotatedCollisionBounds(Vector3 *min, Vector3 *max) const;
			StateFlags GetStateFlags() const;

			Vector3 GetOrigin() const;
			const Vector3 &GetLocalOrigin() const;

			void OriginToWorld(Vector3 *origin) const;
			void OriginToWorld(Vector3 *origin, Quat *rot) const;

			void WorldToOrigin(Vector3 *origin) const;
			void WorldToOrigin(Vector3 *origin, Quat *rot) const;

			virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
			virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

			void SetSleepReportEnabled(bool reportEnabled);
			bool IsSleepReportEnabled() const;
			virtual void OnWake();
			virtual void OnSleep();

			// Should only be called from within an EVENT_INITIALIZE_PHYSICS event!
			PhysObjHandle InitializePhysics(const physics::PhysObjCreateInfo &physObjCreateInfo, PhysFlags flags, int32_t rootMeshBoneId = -1);
		protected:
			BasePhysicsComponent(pragma::ecs::BaseEntity &ent);
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;

			// Mass used for initialization; Not necessarily the same as the mass of the actual physics object
			virtual float GetPhysicsMass() const;
			void UpdateBoneCollisionObject(UInt32 boneId, Bool updatePos = true, Bool updateRot = false);

			pragma::NetEventId m_netEvSetCollisionsEnabled = pragma::INVALID_NET_EVENT;
			pragma::NetEventId m_netEvSetSimEnabled = pragma::INVALID_NET_EVENT;

			bool m_bRayResultCallbackEnabled = false;
			pragma::physics::PHYSICSTYPE m_physicsType = pragma::physics::PHYSICSTYPE::NONE;
			util::TSharedHandle<pragma::physics::PhysObj> m_physObject = nullptr;
			std::vector<PhysJoint> m_joints;
			std::vector<CollisionInfo> m_customCollisions;
			std::vector<CollisionInfo>::iterator FindCollisionInfo(pragma::ecs::BaseEntity *ent);
			util::TSharedHandle<pragma::physics::IRigidBody> CreateRigidBody(pragma::physics::IShape &shape, bool dynamic, const umath::Transform &localPose = {});
			util::TSharedHandle<pragma::physics::PhysObj> InitializeSoftBodyPhysics();
			util::TSharedHandle<pragma::physics::PhysObj> InitializeModelPhysics(PhysFlags flags = PhysFlags::Dynamic);
			util::TSharedHandle<pragma::physics::PhysObj> InitializeBoxControllerPhysics();
			util::TSharedHandle<pragma::physics::PhysObj> InitializeCapsuleControllerPhysics();
			virtual void OnPhysicsInitialized();
			virtual void OnPhysicsDestroyed();
			pragma::physics::CollisionMask m_collisionFilterGroup = pragma::physics::CollisionMask::Default;
			pragma::physics::CollisionMask m_collisionFilterMask = pragma::physics::CollisionMask::Default;
			bool m_bColCallbacksEnabled = false;
			bool m_bColContactReportEnabled = false;
			virtual void InitializePhysObj();
			void UpdatePhysicsBone(Frame &reference, const std::shared_ptr<pragma::animation::Bone> &bone, Quat &invRot, const Vector3 *mvOffset = nullptr);
			void PostPhysicsSimulate(Frame &reference, std::unordered_map<pragma::animation::BoneId, std::shared_ptr<pragma::animation::Bone>> &bones, Vector3 &moveOffset, Quat &invRot, UInt32 physRootBoneId);
			// Updates the entity's bones to match the transforms of the collision objects
			void UpdateRagdollPose();

			StateFlags m_stateFlags = StateFlags::CollisionsEnabled;
			pragma::physics::MOVETYPE m_moveType = {};
			pragma::physics::COLLISIONTYPE m_collisionType = {};
			float m_colRadius = 0.f;
			Vector3 m_colMin = {};
			Vector3 m_colMax = {};
		private:
			void ClearAwakeStatus();
		};
		struct DLLNETWORK CEInitializePhysics : public ComponentEvent {
			CEInitializePhysics(pragma::physics::PHYSICSTYPE type, BasePhysicsComponent::PhysFlags flags);
			virtual void PushArguments(lua_State *l) override;
			pragma::physics::PHYSICSTYPE physicsType;
			BasePhysicsComponent::PhysFlags flags;
		};
		struct DLLNETWORK CEPostPhysicsSimulate : public ComponentEvent {
			CEPostPhysicsSimulate();
			virtual void PushArguments(lua_State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua_State *l) override;
			bool keepAwake = true;
		};
        using namespace umath::scoped_enum::bitwise;
	};
    namespace umath::scoped_enum::bitwise {
        template<>
        struct enable_bitwise_operators<pragma::BasePhysicsComponent::StateFlags> : std::true_type {};
		
        template<>
        struct enable_bitwise_operators<pragma::BasePhysicsComponent::PhysFlags> : std::true_type {};
    }
};

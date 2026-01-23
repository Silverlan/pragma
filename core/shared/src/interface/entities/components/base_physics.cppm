// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

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
					std::weak_ptr<IShape> shape = {};
					math::Transform localPose = {};
				};
				// Returns the number of shapes that have been added
				uint32_t AddShape(IShape &shape, const math::Transform &localPose = {}, BoneId boneId = -1);
				void SetModelMeshBoneMapping(MeshIndex modelMeshIndex, BoneId boneIndex);
				void SetModel(asset::Model &model);
				asset::Model *GetModel() const;
				const std::unordered_map<BoneId, std::vector<ShapeInfo>> &GetShapes() const;
				const std::unordered_map<MeshIndex, BoneId> &GetModelMeshBoneMappings() const;
			  private:
				std::unordered_map<BoneId, std::vector<ShapeInfo>> m_shapes = {};
				std::unordered_map<MeshIndex, BoneId> m_modelMeshIndexToShapeIndex = {};
				util::WeakHandle<asset::Model> m_model = {};
			};
		};
		struct DLLNETWORK CEPhysicsUpdateData : public ComponentEvent {
			CEPhysicsUpdateData(double deltaTime);
			virtual void PushArguments(lua::State *l) override;
			double deltaTime;
		};
		struct DLLNETWORK CEHandleRaycast : public ComponentEvent {
			CEHandleRaycast(physics::CollisionMask rayCollisionGroup, physics::CollisionMask rayCollisionMask);
			virtual void PushArguments(lua::State *l) override;
			physics::CollisionMask rayCollisionGroup;
			physics::CollisionMask rayCollisionMask;
			bool hit = false;
		};
		namespace basePhysicsComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_PHYSICS_INITIALIZED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_PHYSICS_DESTROYED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_PHYSICS_UPDATED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_DYNAMIC_PHYSICS_UPDATED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_PRE_PHYSICS_SIMULATE)
			REGISTER_COMPONENT_EVENT(EVENT_ON_POST_PHYSICS_SIMULATE)
			REGISTER_COMPONENT_EVENT(EVENT_ON_SLEEP)
			REGISTER_COMPONENT_EVENT(EVENT_ON_WAKE)
			REGISTER_COMPONENT_EVENT(EVENT_HANDLE_RAYCAST)
			REGISTER_COMPONENT_EVENT(EVENT_INITIALIZE_PHYSICS)
		}
		class DLLNETWORK BasePhysicsComponent : public BaseEntityComponent {
		  protected:
			struct DLLNETWORK CollisionInfo {
				CollisionInfo();
				CollisionInfo(ecs::BaseEntity *ent, Bool shouldCollide);
				EntityHandle entity;
				Bool shouldCollide;
			};
			struct DLLNETWORK PhysJoint {
				PhysJoint(unsigned int source, unsigned int target);
				unsigned int source; // Bone ID
				unsigned int target; // Bone ID
				util::TSharedHandle<physics::IConstraint> constraint;
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

			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

			virtual void Initialize() override;
			virtual void OnRemove() override;

			const std::vector<PhysJoint> &GetJoints() const;
			std::vector<PhysJoint> &GetJoints();

			bool IsRagdoll() const;
			Vector3 GetCenter() const;

			float GetAABBDistance(const Vector3 &p) const;
			float GetAABBDistance(const ecs::BaseEntity &ent) const;

			virtual bool ShouldCollide(physics::PhysObj *physThis, physics::ICollisionObject *colThis, ecs::BaseEntity *entOther, physics::PhysObj *physOther, physics::ICollisionObject *colOther, bool valDefault) const;
			void ResetCollisions(ecs::BaseEntity *ent);
			void DisableCollisions(ecs::BaseEntity *ent);
			void EnableCollisions(ecs::BaseEntity *ent);
			void SetCollisionsEnabled(ecs::BaseEntity *ent, bool b);
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
			virtual void OnPhysicsWake(physics::PhysObj *phys);
			virtual void OnPhysicsSleep(physics::PhysObj *phys);
			bool IsOnGround() const;
			bool IsGroundWalkable() const;

			void SetForcePhysicsAwakeCallbacksEnabled(bool enabled, bool apply = true, std::optional<bool> isAwakeOverride = {});
			bool AreForcePhysicsAwakeCallbacksEnabled() const;

			ecs::BaseEntity *GetGroundEntity() const;
			physics::PhysObj *GetPhysicsObject() const;
			physics::ICollisionObject *GetCollisionObject(UInt32 boneId) const;
			virtual physics::PhysObj *InitializePhysics(physics::PhysicsType type, PhysFlags flags = PhysFlags::None);
			physics::PhysObj *InitializePhysics(physics::IConvexShape &shape, PhysFlags flags = PhysFlags::None);
			virtual void DestroyPhysicsObject();
			physics::PhysicsType GetPhysicsType() const;
			void DropToFloor();
			bool IsTrigger() const;
			virtual void SetCollisionFilter(physics::CollisionMask filterGroup, physics::CollisionMask filterMask);
			void AddCollisionFilter(physics::CollisionMask filter);
			void RemoveCollisionFilter(physics::CollisionMask filter);
			// Sets both the filterGroup AND filterMask to the specified value
			void SetCollisionFilter(physics::CollisionMask filterGroup);
			void SetCollisionFilterMask(physics::CollisionMask filterMask);
			void SetCollisionFilterGroup(physics::CollisionMask filterGroup);
			physics::CollisionMask GetCollisionFilter() const;
			physics::CollisionMask GetCollisionFilterMask() const;
			void GetCollisionFilter(physics::CollisionMask *filterGroup, physics::CollisionMask *filterMask) const;
			virtual void PhysicsUpdate(double tDelta);
			void RayCast(const Vector3 &dir, float distance) const;
			void Sweep(const Vector3 &dir, float distance) const;
			// Is called after the world physics have been simulated
			virtual void UpdatePhysicsData();
			// Return false to discard result
			virtual bool RayResultCallback(physics::CollisionMask rayCollisionGroup, physics::CollisionMask rayCollisionMask);
			bool IsRayResultCallbackEnabled() const;
			void SetRayResultCallbackEnabled(bool b);

			physics::MoveType GetMoveType() const;
			virtual void SetMoveType(physics::MoveType movetype);
			physics::CollisionType GetCollisionType() const;
			virtual void SetCollisionType(physics::CollisionType collisiontype);

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
			BasePhysicsComponent(ecs::BaseEntity &ent);
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;

			// Mass used for initialization; Not necessarily the same as the mass of the actual physics object
			virtual float GetPhysicsMass() const;
			void UpdateBoneCollisionObject(UInt32 boneId, Bool updatePos = true, Bool updateRot = false);

			NetEventId m_netEvSetCollisionsEnabled = INVALID_NET_EVENT;
			NetEventId m_netEvSetSimEnabled = INVALID_NET_EVENT;

			bool m_bRayResultCallbackEnabled = false;
			physics::PhysicsType m_physicsType = physics::PhysicsType::None;
			util::TSharedHandle<physics::PhysObj> m_physObject = nullptr;
			std::vector<PhysJoint> m_joints;
			std::vector<CollisionInfo> m_customCollisions;
			std::vector<CollisionInfo>::iterator FindCollisionInfo(ecs::BaseEntity *ent);
			util::TSharedHandle<physics::IRigidBody> CreateRigidBody(physics::IShape &shape, bool dynamic, const math::Transform &localPose = {});
			util::TSharedHandle<physics::PhysObj> InitializeSoftBodyPhysics();
			util::TSharedHandle<physics::PhysObj> InitializeModelPhysics(PhysFlags flags = PhysFlags::Dynamic);
			util::TSharedHandle<physics::PhysObj> InitializeBoxControllerPhysics();
			util::TSharedHandle<physics::PhysObj> InitializeCapsuleControllerPhysics();
			virtual void OnPhysicsInitialized();
			virtual void OnPhysicsDestroyed();
			physics::CollisionMask m_collisionFilterGroup = physics::CollisionMask::Default;
			physics::CollisionMask m_collisionFilterMask = physics::CollisionMask::Default;
			bool m_bColCallbacksEnabled = false;
			bool m_bColContactReportEnabled = false;
			virtual void InitializePhysObj();
			void UpdatePhysicsBone(Frame &reference, const std::shared_ptr<animation::Bone> &bone, Quat &invRot, const Vector3 *mvOffset = nullptr);
			void PostPhysicsSimulate(Frame &reference, std::unordered_map<animation::BoneId, std::shared_ptr<animation::Bone>> &bones, Vector3 &moveOffset, Quat &invRot, UInt32 physRootBoneId);
			// Updates the entity's bones to match the transforms of the collision objects
			void UpdateRagdollPose();

			StateFlags m_stateFlags = StateFlags::CollisionsEnabled;
			physics::MoveType m_moveType = {};
			physics::CollisionType m_collisionType = {};
			float m_colRadius = 0.f;
			Vector3 m_colMin = {};
			Vector3 m_colMax = {};
		  private:
			void ClearAwakeStatus();
		};
		struct DLLNETWORK CEInitializePhysics : public ComponentEvent {
			CEInitializePhysics(physics::PhysicsType type, BasePhysicsComponent::PhysFlags flags);
			virtual void PushArguments(lua::State *l) override;
			physics::PhysicsType physicsType;
			BasePhysicsComponent::PhysFlags flags;
		};
		struct DLLNETWORK CEPostPhysicsSimulate : public ComponentEvent {
			CEPostPhysicsSimulate();
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			bool keepAwake = true;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BasePhysicsComponent::StateFlags)
	REGISTER_ENUM_FLAGS(pragma::BasePhysicsComponent::PhysFlags)
};

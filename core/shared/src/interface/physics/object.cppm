// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

#ifdef __linux__
#endif

export module pragma.shared:physics.object;

export import :physics.collision_object;
export import :physics.controller;
export import :physics.controller_hit_data;
export import :physics.object_handle;
export import :scripting.lua.base_lua_handle;
export import :entities.components.handle;

export {
	namespace pragma::ecs {
		class BaseEntity;
	}
	namespace pragma::physics {
		class PhysObj;
	}
	using PhysObjHandle = pragma::util::TWeakSharedHandle<pragma::physics::PhysObj>;

	namespace pragma {
		class BaseEntityComponent;
		class NetworkState;
	};
	namespace pragma::physics {
		class DLLNETWORK PhysObj : public BaseLuaHandle {
		  public:
			enum class StateFlags : uint32_t { None = 0u, Disabled = 1u, Spawned = Disabled << 1u };

			template<class TPhysObj, typename... TARGS>
			static std::unique_ptr<TPhysObj> Create(BaseEntityComponent &owner, TARGS... args);
			template<class TPhysObj, typename... TARGS>
			static std::unique_ptr<TPhysObj> Create(BaseEntityComponent &owner, const std::vector<ICollisionObject *> &objects, TARGS... args);
			virtual ~PhysObj();
			virtual void Spawn();
			virtual void UpdateVelocity();
			virtual BaseEntityComponent *GetOwner();
			NetworkState *GetNetworkState();
			virtual void Enable();
			virtual void Disable();
			bool IsTrigger() const;
			void SetTrigger(bool bTrigger);
			void GetAABB(Vector3 &min, Vector3 &max) const;
			bool IsDisabled() const;
			virtual bool IsStatic() const;
			virtual void SetStatic(bool b);
			virtual float GetMass() const;
			virtual void SetMass(float mass);
			virtual bool IsRigid() const;
			virtual bool IsSoftBody() const;
			virtual void InitializeLuaObject(lua::State *lua) override;
			void SetCCDEnabled(bool b);

			PhysObjHandle GetHandle() const;

			void SetCollisionFilter(CollisionMask filterGroup, CollisionMask filterMask);
			void SetCollisionFilterMask(CollisionMask filterMask);
			void AddCollisionFilter(CollisionMask filter);
			void RemoveCollisionFilter(CollisionMask filter);
			void SetCollisionFilter(CollisionMask filterGroup);
			CollisionMask GetCollisionFilter() const;
			CollisionMask GetCollisionFilterMask() const;
			void GetCollisionFilter(CollisionMask *filterGroup, CollisionMask *filterMask) const;

			virtual void AddCollisionObject(ICollisionObject &o);
			const ICollisionObject *GetCollisionObject() const;
			ICollisionObject *GetCollisionObject();
			const std::vector<util::TSharedHandle<ICollisionObject>> &GetCollisionObjects() const;
			std::vector<util::TSharedHandle<ICollisionObject>> &GetCollisionObjects();

			virtual Vector3 GetLinearVelocity() const;
			virtual void SetLinearVelocity(const Vector3 &vel);
			void AddLinearVelocity(const Vector3 &vel);
			virtual Vector3 GetAngularVelocity() const;
			virtual void SetAngularVelocity(const Vector3 &vel);
			void AddAngularVelocity(const Vector3 &vel);
			virtual void SetPosition(const Vector3 &pos);
			virtual void SetOrientation(const Quat &q);
			virtual Quat GetOrientation() const;
			virtual Vector3 GetPosition() const;
			Vector3 GetOrigin() const;
			uint32_t GetNumberOfCollisionObjectsAwake() const;

			virtual void PutToSleep();
			virtual void WakeUp();
			virtual bool IsSleeping() const;
			virtual void Simulate(double tDelta, bool bIgnoreGravity = false);
			virtual bool IsController() const;
			virtual void OnSleep();
			virtual void OnWake();

			virtual void SetLinearFactor(const Vector3 &factor);
			virtual void SetAngularFactor(const Vector3 &factor);
			virtual Vector3 GetLinearFactor() const;
			virtual Vector3 GetAngularFactor() const;

			virtual void SetDamping(float linDamping, float angDamping);
			virtual void SetLinearDamping(float damping);
			virtual void SetAngularDamping(float damping);
			virtual float GetLinearDamping() const;
			virtual float GetAngularDamping() const;

			virtual void ApplyForce(const Vector3 &force);
			virtual void ApplyForce(const Vector3 &force, const Vector3 &relPos);
			virtual void ApplyImpulse(const Vector3 &impulse);
			virtual void ApplyImpulse(const Vector3 &impulse, const Vector3 &relPos);
			virtual void ApplyTorque(const Vector3 &torque);
			virtual void ApplyTorqueImpulse(const Vector3 &torque);
			virtual void ClearForces();
			virtual Vector3 GetTotalForce() const;
			virtual Vector3 GetTotalTorque() const;

			void SetLinearSleepingThreshold(float threshold);
			void SetAngularSleepingThreshold(float threshold);
			virtual void SetSleepingThresholds(float linear, float angular);
			virtual float GetLinearSleepingThreshold() const;
			virtual float GetAngularSleepingThreshold() const;
			std::pair<float, float> GetSleepingThreshold() const;
		  protected:
			friend ICollisionObject;
			PhysObj(BaseEntityComponent *owner, ICollisionObject &object);
			PhysObj(BaseEntityComponent *owner, const std::vector<ICollisionObject *> &objects);
			PhysObj(BaseEntityComponent *owner);
			bool Initialize();
			void OnCollisionObjectWake(ICollisionObject &o);
			void OnCollisionObjectSleep(ICollisionObject &o);
			void OnCollisionObjectRemoved(ICollisionObject &o);

			Vector3 m_velocity = {};
			std::vector<util::TSharedHandle<ICollisionObject>> m_collisionObjects;

			ComponentHandle<BaseEntityComponent> m_owner = {};
			NetworkState *m_networkState;
			CollisionMask m_collisionFilterGroup = {};
			CollisionMask m_collisionFilterMask = {};
			StateFlags m_stateFlags = StateFlags::None;
			uint32_t m_colObjAwakeCount = 0u;
		};

		template<class TPhysObj, typename... TARGS>
		std::unique_ptr<TPhysObj> PhysObj::Create(BaseEntityComponent &owner, TARGS... args)
		{
			auto physObj = std::unique_ptr<TPhysObj> {new TPhysObj {&owner}};
			if(physObj->Initialize(args...) == false)
				return nullptr;
			return physObj;
		}

		template<class TPhysObj, typename... TARGS>
		std::unique_ptr<TPhysObj> PhysObj::Create(BaseEntityComponent &owner, const std::vector<ICollisionObject *> &objects, TARGS... args)
		{
			auto physObj = std::unique_ptr<TPhysObj> {new TPhysObj {&owner}};
			if(physObj->Initialize(args...) == false)
				return nullptr;
			for(auto *o : objects)
				physObj->AddCollisionObject(*o);
			return physObj;
		}

		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::physics::PhysObj::StateFlags)

	DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::physics::PhysObj &o);

	////////////////////////////////////

	namespace pragma::physics {
		class DLLNETWORK PhysObjDynamic {
		public:
			friend PhysObj;
			virtual void PreSimulate();
			virtual void PostSimulate();
		protected:
#if PHYS_KEEP_SIMULATION_TRANSFORM != 0
			std::vector<PhysTransform> m_offsets;
		public:
			Vector3 GetSimulationOffset(unsigned int idx = 0);
			Quat GetSimulationRotation(unsigned int idx = 0);
#endif
		};

		////////////////////////////////////

		class DLLNETWORK PhysObjKinematic {
		public:
			virtual void SetKinematic(bool b) = 0;
			bool IsKinematic() const;
		protected:
			PhysObjKinematic();
			bool m_bKinematic = false;
		};

		////////////////////////////////////

		class DLLNETWORK SoftBodyPhysObj : public PhysObj, public PhysObjDynamic {
		public:
			friend PhysObj;
			std::vector<util::TSharedHandle<ISoftBody>> &GetSoftBodies();
			const ISoftBody *GetSoftBody() const;
			ISoftBody *GetSoftBody();
			virtual bool IsStatic() const override;
			virtual bool IsSoftBody() const override;
			virtual void AddCollisionObject(ICollisionObject &o) override;
			virtual void SetLinearVelocity(const Vector3 &vel) override;
			virtual BaseEntityComponent *GetOwner() override;

			virtual float GetMass() const override;
			virtual void SetMass(float mass) override;
			virtual void Simulate(double tDelta, bool bIgnoreGravity = false) override;
			virtual void PutToSleep() override;
			virtual void WakeUp() override;
			virtual bool IsSleeping() const override;

			virtual void ApplyForce(const Vector3 &force) override;
		protected:
			SoftBodyPhysObj(BaseEntityComponent *owner);
			bool Initialize(ISoftBody &body);
			bool Initialize(const std::vector<ISoftBody *> &bodies);
			std::vector<util::TSharedHandle<ISoftBody>> m_softBodies;
		};

		////////////////////////////////////

		class DLLNETWORK RigidPhysObj : public PhysObj, public PhysObjKinematic, public PhysObjDynamic {
		public:
			friend PhysObj;
			virtual ~RigidPhysObj() override;
			std::vector<util::TSharedHandle<IRigidBody>> &GetRigidBodies();
			virtual void UpdateVelocity() override;
			virtual void AddCollisionObject(ICollisionObject &o) override;
			const IRigidBody *GetRigidBody() const;
			IRigidBody *GetRigidBody();
			virtual bool IsStatic() const override;
			virtual void SetStatic(bool b) override;
			virtual float GetMass() const override;
			virtual void SetMass(float mass) override;
			virtual bool IsRigid() const override;
			virtual void SetKinematic(bool b) override;
			virtual void Simulate(double tDelta, bool bIgnoreGravity = false) override;
			virtual BaseEntityComponent *GetOwner() override;

			virtual Vector3 GetLinearVelocity() const override;
			virtual void SetLinearVelocity(const Vector3 &vel) override;
			virtual Vector3 GetAngularVelocity() const override;
			virtual void SetAngularVelocity(const Vector3 &vel) override;
			virtual void PutToSleep() override;
			virtual void WakeUp() override;
			virtual bool IsSleeping() const override;
			virtual void OnSleep() override;
			virtual void OnWake() override;

			virtual void SetLinearFactor(const Vector3 &factor) override;
			virtual void SetAngularFactor(const Vector3 &factor) override;
			virtual Vector3 GetLinearFactor() const override;
			virtual Vector3 GetAngularFactor() const override;

			virtual void SetDamping(float linDamping, float angDamping) override;
			virtual void SetLinearDamping(float damping) override;
			virtual void SetAngularDamping(float damping) override;
			virtual float GetLinearDamping() const override;
			virtual float GetAngularDamping() const override;

			virtual void ApplyForce(const Vector3 &force) override;
			virtual void ApplyForce(const Vector3 &force, const Vector3 &relPos) override;
			virtual void ApplyImpulse(const Vector3 &impulse) override;
			virtual void ApplyImpulse(const Vector3 &impulse, const Vector3 &relPos) override;
			virtual void ApplyTorque(const Vector3 &torque) override;
			virtual void ApplyTorqueImpulse(const Vector3 &torque) override;
			virtual void ClearForces() override;
			virtual Vector3 GetTotalForce() const override;
			virtual Vector3 GetTotalTorque() const override;

			virtual void SetSleepingThresholds(float linear, float angular) override;
			virtual float GetLinearSleepingThreshold() const override;
			virtual float GetAngularSleepingThreshold() const override;
		protected:
			RigidPhysObj(BaseEntityComponent *owner);
			bool Initialize(IRigidBody &body);
			bool Initialize(const std::vector<IRigidBody *> &bodies);
			std::vector<util::TSharedHandle<IRigidBody>> m_rigidBodies;
			float m_mass = 0.f;
			bool m_bStatic = false;
			void ApplyMass(float mass);
		};

		////////////////////////////////////

		class DLLNETWORK ControllerPhysObj : public PhysObj, public PhysObjKinematic, public PhysObjDynamic {
		public:
			friend PhysObj;
			virtual ~ControllerPhysObj() override;
			IController *GetController();
			ICollisionObject *GetCollisionObject();
			virtual void PostSimulate() override;
			float GetStepHeight() const;
			virtual void SetKinematic(bool b) override;
			virtual void SetLinearVelocity(const Vector3 &vel) override;
			Vector3 &GetOffset();
			void SetOffset(const Vector3 &offset);
			void Simulate(double tDelta, bool bIgnoreGravity = false);
			bool IsController() const;
			virtual bool IsCapsule() const;
			math::Degree GetSlopeLimit() const;
			void SetSlopeLimit(math::Degree limit);
			float GetStepOffset() const;
			void SetStepOffset(float offset);
			virtual void SetCollisionBounds(const Vector3 &min, const Vector3 &max);
			virtual void GetCollisionBounds(Vector3 *min, Vector3 *max);
			BaseEntityComponent *GetOwner();
			virtual void SetOrientation(const Quat &q) override;
			virtual void SetPosition(const Vector3 &pos) override;
			virtual Vector3 GetPosition() const override;
			virtual void UpdateVelocity() override;
			unsigned int Move(const Vector3 &disp, float elapsedTime, float minDist = 0.1f);
			ControllerHitData &GetControllerHitData();

			bool IsOnGround() const;
			bool IsGroundWalkable() const;
			ecs::BaseEntity *GetGroundEntity() const;
			PhysObj *GetGroundPhysObject() const;
			int32_t GetGroundSurfaceMaterial() const;
			IMaterial *GetGroundMaterial() const;
			const ICollisionObject *GetGroundPhysCollisionObject() const;
			ICollisionObject *GetGroundPhysCollisionObject();
			// The velocity affecting this controller originating from the ground object
			Vector3 GetGroundVelocity() const;

			Vector3 GetDimensions() const;
			void SetDimensions(const Vector3 &dimensions);
		protected:
			ControllerHitData m_hitData = {};
			Vector3 m_offset = {};
			double m_tLastMove = 0.0;

			util::TSharedHandle<IController> m_controller = nullptr;
			util::TSharedHandle<ICollisionObject> m_collisionObject = nullptr;
			Vector3 m_originLast = {0.f, 0.f, 0.f};
			float m_stepHeight = 0.f;
			ControllerPhysObj(BaseEntityComponent *owner);
		};

		class DLLNETWORK BoxControllerPhysObj : public ControllerPhysObj {
		public:
			friend PhysObj;
			Vector3 &GetHalfExtents();
			void SetCollisionBounds(const Vector3 &min, const Vector3 &max);
			void GetCollisionBounds(Vector3 *min, Vector3 *max);
		protected:
			BoxControllerPhysObj(BaseEntityComponent *owner);
			bool Initialize(const Vector3 &halfExtents, unsigned int stepHeight, float maxSlopeDeg = 45.f);
			Vector3 m_halfExtents = {};
		};

		class DLLNETWORK CapsuleControllerPhysObj : public ControllerPhysObj {
		public:
			friend PhysObj;
			float GetWidth() const;
			float GetHeight() const;
			void SetHeight(float height);
			virtual bool IsCapsule() const override;
			void SetCollisionBounds(const Vector3 &min, const Vector3 &max);
			void GetCollisionBounds(Vector3 *min, Vector3 *max);
		protected:
			CapsuleControllerPhysObj(BaseEntityComponent *owner);
			bool Initialize(unsigned int width, unsigned int height, unsigned int stepHeight, float maxSlopeDeg = 45.f);
			float m_width = 0.f;
			float m_height = 0.f;
		};
	}
};

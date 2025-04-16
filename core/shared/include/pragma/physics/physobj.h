/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYSOBJ_H__
#define __PHYSOBJ_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <memory>
#include <vector>
#include <optional>
#include <mathutil/uquat.h>
#include <sharedutils/def_handle.h>
#include "pragma/entities/components/basegravity.h"
#include "pragma/physics/controllerhitdata.h"
#include <mathutil/transform.hpp>
#include "pragma/physics/collision_object.hpp"
#include "pragma/types.hpp"
#ifdef __linux__
#include "pragma/physics/controller.hpp"
#endif

// Very expensive
#define PHYS_KEEP_SIMULATION_TRANSFORM 0

namespace pragma {
	class BaseEntityComponent;
	namespace physics {
		class IController;
		class IGhostObject;
	};
};

using PhysObjHandle = util::TWeakSharedHandle<PhysObj>;

enum class CollisionMask : uint32_t;
class DLLNETWORK PhysObj : public pragma::BaseLuaHandle {
  public:
	enum class StateFlags : uint32_t { None = 0u, Disabled = 1u, Spawned = Disabled << 1u };

	template<class TPhysObj, typename... TARGS>
	static std::unique_ptr<TPhysObj> Create(pragma::BaseEntityComponent &owner, TARGS... args);
	template<class TPhysObj, typename... TARGS>
	static std::unique_ptr<TPhysObj> Create(pragma::BaseEntityComponent &owner, const std::vector<pragma::physics::ICollisionObject *> &objects, TARGS... args);
	virtual ~PhysObj();
	virtual void Spawn();
	virtual void UpdateVelocity();
	virtual pragma::BaseEntityComponent *GetOwner();
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
	virtual void InitializeLuaObject(lua_State *lua) override;
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

	virtual void AddCollisionObject(pragma::physics::ICollisionObject &o);
	const pragma::physics::ICollisionObject *GetCollisionObject() const;
	pragma::physics::ICollisionObject *GetCollisionObject();
	const std::vector<util::TSharedHandle<pragma::physics::ICollisionObject>> &GetCollisionObjects() const;
	std::vector<util::TSharedHandle<pragma::physics::ICollisionObject>> &GetCollisionObjects();

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
	friend pragma::physics::ICollisionObject;
	PhysObj(pragma::BaseEntityComponent *owner, pragma::physics::ICollisionObject &object);
	PhysObj(pragma::BaseEntityComponent *owner, const std::vector<pragma::physics::ICollisionObject *> &objects);
	PhysObj(pragma::BaseEntityComponent *owner);
	bool Initialize();
	void OnCollisionObjectWake(pragma::physics::ICollisionObject &o);
	void OnCollisionObjectSleep(pragma::physics::ICollisionObject &o);
	void OnCollisionObjectRemoved(pragma::physics::ICollisionObject &o);

	Vector3 m_velocity = {};
	std::vector<util::TSharedHandle<pragma::physics::ICollisionObject>> m_collisionObjects;

	pragma::ComponentHandle<pragma::BaseEntityComponent> m_owner = {};
	NetworkState *m_networkState;
	CollisionMask m_collisionFilterGroup = {};
	CollisionMask m_collisionFilterMask = {};
	StateFlags m_stateFlags = StateFlags::None;
	uint32_t m_colObjAwakeCount = 0u;
};
REGISTER_BASIC_BITWISE_OPERATORS(PhysObj::StateFlags)

DLLNETWORK std::ostream &operator<<(std::ostream &out, const PhysObj &o);

template<class TPhysObj, typename... TARGS>
std::unique_ptr<TPhysObj> PhysObj::Create(pragma::BaseEntityComponent &owner, TARGS... args)
{
	auto physObj = std::unique_ptr<TPhysObj> {new TPhysObj {&owner}};
	if(physObj->Initialize(args...) == false)
		return nullptr;
	return physObj;
}

template<class TPhysObj, typename... TARGS>
std::unique_ptr<TPhysObj> PhysObj::Create(pragma::BaseEntityComponent &owner, const std::vector<pragma::physics::ICollisionObject *> &objects, TARGS... args)
{
	auto physObj = std::unique_ptr<TPhysObj> {new TPhysObj {&owner}};
	if(physObj->Initialize(args...) == false)
		return nullptr;
	for(auto *o : objects)
		physObj->AddCollisionObject(*o);
	return physObj;
}

////////////////////////////////////

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
	std::vector<util::TSharedHandle<pragma::physics::ISoftBody>> &GetSoftBodies();
	const pragma::physics::ISoftBody *GetSoftBody() const;
	pragma::physics::ISoftBody *GetSoftBody();
	virtual bool IsStatic() const override;
	virtual bool IsSoftBody() const override;
	virtual void AddCollisionObject(pragma::physics::ICollisionObject &o) override;
	virtual void SetLinearVelocity(const Vector3 &vel) override;
	virtual pragma::BaseEntityComponent *GetOwner() override;

	virtual float GetMass() const override;
	virtual void SetMass(float mass) override;
	virtual void Simulate(double tDelta, bool bIgnoreGravity = false) override;
	virtual void PutToSleep() override;
	virtual void WakeUp() override;
	virtual bool IsSleeping() const override;

	virtual void ApplyForce(const Vector3 &force) override;
  protected:
	SoftBodyPhysObj(pragma::BaseEntityComponent *owner);
	bool Initialize(pragma::physics::ISoftBody &body);
	bool Initialize(const std::vector<pragma::physics::ISoftBody *> &bodies);
	std::vector<util::TSharedHandle<pragma::physics::ISoftBody>> m_softBodies;
};

////////////////////////////////////

class DLLNETWORK RigidPhysObj : public PhysObj, public PhysObjKinematic, public PhysObjDynamic {
  public:
	friend PhysObj;
	virtual ~RigidPhysObj() override;
	std::vector<util::TSharedHandle<pragma::physics::IRigidBody>> &GetRigidBodies();
	virtual void UpdateVelocity() override;
	virtual void AddCollisionObject(pragma::physics::ICollisionObject &o) override;
	const pragma::physics::IRigidBody *GetRigidBody() const;
	pragma::physics::IRigidBody *GetRigidBody();
	virtual bool IsStatic() const override;
	virtual void SetStatic(bool b) override;
	virtual float GetMass() const override;
	virtual void SetMass(float mass) override;
	virtual bool IsRigid() const override;
	virtual void SetKinematic(bool b) override;
	virtual void Simulate(double tDelta, bool bIgnoreGravity = false) override;
	virtual pragma::BaseEntityComponent *GetOwner() override;

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
	RigidPhysObj(pragma::BaseEntityComponent *owner);
	bool Initialize(pragma::physics::IRigidBody &body);
	bool Initialize(const std::vector<pragma::physics::IRigidBody *> &bodies);
	std::vector<util::TSharedHandle<pragma::physics::IRigidBody>> m_rigidBodies;
	float m_mass = 0.f;
	bool m_bStatic = false;
	void ApplyMass(float mass);
};

////////////////////////////////////

namespace pragma::physics {
	class IMaterial;
};
class DLLNETWORK ControllerPhysObj : public PhysObj, public PhysObjKinematic, public PhysObjDynamic {
  public:
	friend PhysObj;
	virtual ~ControllerPhysObj() override;
	pragma::physics::IController *GetController();
	pragma::physics::ICollisionObject *GetCollisionObject();
	virtual void PostSimulate() override;
	float GetStepHeight() const;
	virtual void SetKinematic(bool b) override;
	virtual void SetLinearVelocity(const Vector3 &vel) override;
	Vector3 &GetOffset();
	void SetOffset(const Vector3 &offset);
	void Simulate(double tDelta, bool bIgnoreGravity = false);
	bool IsController() const;
	virtual bool IsCapsule() const;
	umath::Degree GetSlopeLimit() const;
	void SetSlopeLimit(umath::Degree limit);
	float GetStepOffset() const;
	void SetStepOffset(float offset);
	virtual void SetCollisionBounds(const Vector3 &min, const Vector3 &max);
	virtual void GetCollisionBounds(Vector3 *min, Vector3 *max);
	pragma::BaseEntityComponent *GetOwner();
	virtual void SetOrientation(const Quat &q) override;
	virtual void SetPosition(const Vector3 &pos) override;
	virtual Vector3 GetPosition() const override;
	virtual void UpdateVelocity() override;
	unsigned int Move(const Vector3 &disp, float elapsedTime, float minDist = 0.1f);
	ControllerHitData &GetControllerHitData();

	bool IsOnGround() const;
	bool IsGroundWalkable() const;
	BaseEntity *GetGroundEntity() const;
	PhysObj *GetGroundPhysObject() const;
	int32_t GetGroundSurfaceMaterial() const;
	pragma::physics::IMaterial *GetGroundMaterial() const;
	const pragma::physics::ICollisionObject *GetGroundPhysCollisionObject() const;
	pragma::physics::ICollisionObject *GetGroundPhysCollisionObject();
	// The velocity affecting this controller originating from the ground object
	Vector3 GetGroundVelocity() const;

	Vector3 GetDimensions() const;
	void SetDimensions(const Vector3 &dimensions);
  protected:
	ControllerHitData m_hitData = {};
	Vector3 m_offset = {};
	double m_tLastMove = 0.0;

	util::TSharedHandle<pragma::physics::IController> m_controller = nullptr;
	util::TSharedHandle<pragma::physics::ICollisionObject> m_collisionObject = nullptr;
	Vector3 m_originLast = {0.f, 0.f, 0.f};
	float m_stepHeight = 0.f;
	ControllerPhysObj(pragma::BaseEntityComponent *owner);
};

class DLLNETWORK BoxControllerPhysObj : public ControllerPhysObj {
  public:
	friend PhysObj;
	Vector3 &GetHalfExtents();
	void SetCollisionBounds(const Vector3 &min, const Vector3 &max);
	void GetCollisionBounds(Vector3 *min, Vector3 *max);
  protected:
	BoxControllerPhysObj(pragma::BaseEntityComponent *owner);
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
	CapsuleControllerPhysObj(pragma::BaseEntityComponent *owner);
	bool Initialize(unsigned int width, unsigned int height, unsigned int stepHeight, float maxSlopeDeg = 45.f);
	float m_width = 0.f;
	float m_height = 0.f;
};

#endif

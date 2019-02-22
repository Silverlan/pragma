#ifndef __PHYSOBJ_H__
#define __PHYSOBJ_H__

#include "pragma/networkdefinitions.h"
#include <pragma/physics/physapi.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "pragma/physics/physkinematiccharactercontroller.h"
#include <mathutil/glmutil.h>
#include <memory>
#include <vector>
#include <optional>
#include <mathutil/uquat.h>
#include <sharedutils/def_handle.h>
#include "pragma/entities/components/basegravity.h"
#include "pragma/physics/controllerhitdata.h"
#include "pragma/physics/phystransform.h"
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/phys_contact_info.hpp"
#ifdef __linux__
#include "pragma/physics/physcontroller.h"
#endif

class DLLNETWORK PhysObj;
DECLARE_BASE_HANDLE(DLLNETWORK,PhysObj,PhysObj);

// Very expensive
#define PHYS_KEEP_SIMULATION_TRANSFORM 0

class EntityHandle;
class PhysRigidBody;
class PhysConvexShape;
class PhysController;
class PhysGhostObject;
namespace pragma {class BaseEntityComponent;};
enum class CollisionMask : uint32_t;
class DLLNETWORK PhysObj
{
protected:
	PhysObj(pragma::BaseEntityComponent *owner);
#ifdef PHYS_ENGINE_PHYSX
	std::vector<physx::PxRigidActor*> m_actors;
	PhysObj(BaseEntity *owner,physx::PxRigidActor *actor);
	PhysObj(BaseEntity *owner,std::vector<physx::PxRigidActor*> *actors);
#endif
#ifdef PHYS_ENGINE_BULLET
	Vector3 m_velocity = {};
	std::vector<PhysCollisionObjectHandle> m_collisionObjects;
#endif
	util::WeakHandle<pragma::BaseEntityComponent> m_owner = {};
	NetworkState *m_networkState;
	bool m_bAsleep = true;
	bool m_bDisabled = false;
	bool m_bTrigger = false;
	bool m_bSpawned = false;
	CollisionMask m_collisionFilterGroup = {};
	CollisionMask m_collisionFilterMask = {};
	PhysObjHandle m_handle = {};
	void UpdateCCD();
public:
#ifdef PHYS_ENGINE_BULLET
	PhysObj(pragma::BaseEntityComponent *owner,PhysCollisionObject *object);
	PhysObj(pragma::BaseEntityComponent *owner,std::vector<PhysCollisionObject*> *objects);
#endif
	virtual ~PhysObj();
	virtual void Spawn();
	virtual void UpdateVelocity();
	PhysObjHandle GetHandle();
	PhysObjHandle *CreateHandle();
	virtual pragma::BaseEntityComponent *GetOwner();
	NetworkState *GetNetworkState();
	virtual void Enable();
	virtual void Disable();
	void SetTrigger(bool b);
	bool IsTrigger();
	void GetAABB(Vector3 &min,Vector3 &max) const;
	bool IsDisabled() const;
	virtual bool IsStatic() const;
	virtual void SetStatic(bool b);
	virtual float GetMass() const;
	virtual void SetMass(float mass);
	virtual bool IsRigid() const;
	virtual bool IsSoftBody() const;
	void SetCCDEnabled(bool b);

	void SetCollisionFilter(CollisionMask filterGroup,CollisionMask filterMask);
	void SetCollisionFilterMask(CollisionMask filterMask);
	void AddCollisionFilter(CollisionMask filter);
	void RemoveCollisionFilter(CollisionMask filter);
	void SetCollisionFilter(CollisionMask filterGroup);
	CollisionMask GetCollisionFilter();
	CollisionMask GetCollisionFilterMask();
	void GetCollisionFilter(CollisionMask *filterGroup,CollisionMask *filterMask);

#ifdef PHYS_ENGINE_PHYSX
	// Returns the first actor
	virtual void AddActor(physx::PxRigidActor *actor);
	physx::PxRigidActor *GetActor();
	std::vector<physx::PxRigidActor*> *GetActors();
#endif
#ifdef PHYS_ENGINE_BULLET
	virtual void AddCollisionObject(PhysCollisionObject *o);
	PhysCollisionObject *GetCollisionObject();
	std::vector<PhysCollisionObjectHandle> &GetCollisionObjects();
#endif
	virtual Vector3 GetLinearVelocity();
	virtual void SetLinearVelocity(const Vector3 &vel);
	void AddLinearVelocity(const Vector3 &vel);
	virtual Vector3 GetAngularVelocity();
	virtual void SetAngularVelocity(const Vector3 &vel);
	void AddAngularVelocity(const Vector3 &vel);
	virtual void SetPosition(const Vector3 &pos);
	virtual void SetOrientation(const Quat &q);
	virtual Quat GetOrientation();
	virtual Vector3 GetPosition();
	Vector3 GetOrigin() const;
	
	virtual void PutToSleep();
	virtual void WakeUp();
	virtual bool IsSleeping();
	virtual void Simulate(double tDelta,bool bIgnoreGravity=false);
	virtual bool IsController();
	virtual void OnSleep();
	virtual void OnWake();

	virtual void SetLinearFactor(const Vector3 &factor);
	virtual void SetAngularFactor(const Vector3 &factor);
	virtual Vector3 GetLinearFactor() const;
	virtual Vector3 GetAngularFactor() const;

	virtual void SetDamping(float linDamping,float angDamping);
	virtual void SetLinearDamping(float damping);
	virtual void SetAngularDamping(float damping);
	virtual float GetLinearDamping() const;
	virtual float GetAngularDamping() const;

	virtual void ApplyForce(const Vector3 &force);
	virtual void ApplyForce(const Vector3 &force,const Vector3 &relPos);
	virtual void ApplyImpulse(const Vector3 &impulse);
	virtual void ApplyImpulse(const Vector3 &impulse,const Vector3 &relPos);
	virtual void ApplyTorque(const Vector3 &torque);
	virtual void ApplyTorqueImpulse(const Vector3 &torque);
	virtual void ClearForces();
	virtual Vector3 GetTotalForce();
	virtual Vector3 GetTotalTorque();

	void SetLinearSleepingThreshold(float threshold);
	void SetAngularSleepingThreshold(float threshold);
	virtual void SetSleepingThresholds(float linear,float angular);
	virtual float GetLinearSleepingThreshold() const;
	virtual float GetAngularSleepingThreshold() const;
	std::pair<float,float> GetSleepingThreshold() const;
};

////////////////////////////////////

class DLLNETWORK PhysObjDynamic
{
protected:
#if PHYS_KEEP_SIMULATION_TRANSFORM != 0
#ifdef PHYS_ENGINE_BULLET
	std::vector<PhysTransform> m_offsets;
#elif PHYS_ENGINE_PHYSX
	std::vector<physx::PxTransform> m_offsets;
#endif
public:
	Vector3 GetSimulationOffset(unsigned int idx=0);
	Quat GetSimulationRotation(unsigned int idx=0);
#endif
public:
	virtual void PreSimulate();
	virtual void PostSimulate();
};

////////////////////////////////////

class DLLNETWORK PhysObjKinematic
{
protected:
	bool m_bKinematic = false;
#ifdef PHYS_ENGINE_PHYSX
	void SetKinematic(bool b,std::vector<physx::PxRigidActor*> &actors);
#endif
public:
	PhysObjKinematic();
	virtual void SetKinematic(bool b)=0;
	bool IsKinematic();
};

////////////////////////////////////

class DLLNETWORK SoftBodyPhysObj
	: public PhysObj,public PhysObjDynamic
{
protected:
	std::vector<PhysSoftBodyHandle> m_softBodies;
public:
	SoftBodyPhysObj(pragma::BaseEntityComponent *owner,PhysSoftBody *body);
	SoftBodyPhysObj(pragma::BaseEntityComponent *owner,std::vector<PhysSoftBody*> *bodies);
	std::vector<PhysSoftBodyHandle> &GetSoftBodies();
	PhysSoftBody *GetSoftBody() const;
	virtual bool IsStatic() const override;
	virtual bool IsSoftBody() const override;
	virtual void AddCollisionObject(PhysCollisionObject *o) override;
	virtual void SetLinearVelocity(const Vector3 &vel) override;
	virtual pragma::BaseEntityComponent *GetOwner() override;

	virtual float GetMass() const override;
	virtual void SetMass(float mass) override;
	virtual void Simulate(double tDelta,bool bIgnoreGravity=false) override;
	virtual void PutToSleep() override;
	virtual void WakeUp() override;
	virtual bool IsSleeping() override;

	virtual void ApplyForce(const Vector3 &force) override;
};

////////////////////////////////////

class DLLNETWORK RigidPhysObj
	: public PhysObj,public PhysObjKinematic,public PhysObjDynamic
{
protected:
	std::vector<PhysRigidBodyHandle> m_rigidBodies;
	float m_mass = 0.f;
	bool m_bStatic = false;
	void ApplyMass(float mass);
public:
	RigidPhysObj(pragma::BaseEntityComponent *owner,PhysRigidBody *body);
	RigidPhysObj(pragma::BaseEntityComponent *owner,std::vector<PhysRigidBody*> *bodies);
	virtual ~RigidPhysObj() override;
	std::vector<PhysRigidBodyHandle> &GetRigidBodies();
	virtual void UpdateVelocity() override;
	virtual void AddCollisionObject(PhysCollisionObject *o) override;
	PhysRigidBody *GetRigidBody() const;
	virtual bool IsStatic() const override;
	virtual void SetStatic(bool b) override;
	virtual float GetMass() const override;
	virtual void SetMass(float mass) override;
	virtual bool IsRigid() const override;
	virtual void SetKinematic(bool b) override;
	virtual void Simulate(double tDelta,bool bIgnoreGravity=false) override;
	virtual pragma::BaseEntityComponent *GetOwner() override;

	virtual Vector3 GetLinearVelocity() override;
	virtual void SetLinearVelocity(const Vector3 &vel) override;
	virtual Vector3 GetAngularVelocity() override;
	virtual void SetAngularVelocity(const Vector3 &vel) override;
	virtual void PutToSleep() override;
	virtual void WakeUp() override;
	virtual bool IsSleeping() override;
	virtual void OnSleep() override;
	virtual void OnWake() override;

	virtual void SetLinearFactor(const Vector3 &factor) override;
	virtual void SetAngularFactor(const Vector3 &factor) override;
	virtual Vector3 GetLinearFactor() const override;
	virtual Vector3 GetAngularFactor() const override;

	virtual void SetDamping(float linDamping,float angDamping) override;
	virtual void SetLinearDamping(float damping) override;
	virtual void SetAngularDamping(float damping) override;
	virtual float GetLinearDamping() const override;
	virtual float GetAngularDamping() const override;

	virtual void ApplyForce(const Vector3 &force) override;
	virtual void ApplyForce(const Vector3 &force,const Vector3 &relPos) override;
	virtual void ApplyImpulse(const Vector3 &impulse) override;
	virtual void ApplyImpulse(const Vector3 &impulse,const Vector3 &relPos) override;
	virtual void ApplyTorque(const Vector3 &torque) override;
	virtual void ApplyTorqueImpulse(const Vector3 &torque) override;
	virtual void ClearForces() override;
	virtual Vector3 GetTotalForce() override;
	virtual Vector3 GetTotalTorque() override;

	virtual void SetSleepingThresholds(float linear,float angular) override;
	virtual float GetLinearSleepingThreshold() const override;
	virtual float GetAngularSleepingThreshold() const override;
};

////////////////////////////////////

class DLLNETWORK ControllerPhysObj
	: public PhysObj,public PhysObjKinematic,public PhysObjDynamic
{
protected:
	struct GroundInfo
	{
		GroundInfo(const btManifoldPoint &contactPoint,int8_t controllerIndex)
			: contactInfo{contactPoint,controllerIndex}
		{}
		PhysContactInfo contactInfo;
		bool groundWalkable = false;
		double contactDistance = std::numeric_limits<double>::max(); // Distance on XZ plane; Used to determine best contact point candidate
		double minContactDistance = std::numeric_limits<double>::max(); // Minimum XZ distance for ALL contact points (in this tick)
	};
	std::optional<GroundInfo> m_groundInfo {};
	ControllerHitData m_hitData = {};
	Vector3 m_offset = {};
	double m_tLastMove = 0.0;
	Float m_currentFriction = 1.f;
#ifdef PHYS_ENGINE_BULLET
	std::unique_ptr<PhysController> m_controller = nullptr;
	PhysGhostObject *m_ghostObject = nullptr;
	Vector3 m_posLast = {0.f,0.f,0.f};
	Vector3 m_originLast = {0.f,0.f,0.f};
	float m_stepHeight = 0.f;
	ControllerPhysObj(pragma::BaseEntityComponent *owner);
#elif PHYS_ENGINE_PHYSX
	physx::PxController *m_controller = nullptr;
	physx::PxVec3 m_velocity;
	physx::PxVec3 m_posLast;
	ControllerPhysObj(BaseEntity *owner,physx::PxController *controller);
#endif
public:
	virtual ~ControllerPhysObj() override;
#ifdef PHYS_ENGINE_BULLET
	PhysController *GetController();
	PhysGhostObject *GetGhostObject();
#elif PHYS_ENGINE_PHYSX
	unsigned int Move(const Vector3 &disp,float elapsedTime,float minDist,const physx::PxControllerFilters &filters);
	virtual physx::PxController *GetController() override;
#endif
	virtual void PostSimulate() override;
	float GetStepHeight();
	void SetMaxSlope(float ang);
	float GetMaxSlope();
	virtual void SetKinematic(bool b) override;
	virtual void SetLinearVelocity(const Vector3 &vel) override;
	Vector3 &GetOffset();
	void SetOffset(const Vector3 &offset);
	void Simulate(double tDelta,bool bIgnoreGravity=false);
	bool IsController();
	virtual bool IsCapsule();
	float GetStepOffset();
	void SetStepOffset(float offset);
	virtual void SetCollisionBounds(const Vector3 &min,const Vector3 &max);
	virtual void GetCollisionBounds(Vector3 *min,Vector3 *max);
	pragma::BaseEntityComponent *GetOwner();
	virtual void SetOrientation(const Quat &q) override;
	virtual void SetPosition(const Vector3 &pos) override;
	virtual Vector3 GetPosition() override;
	virtual void UpdateVelocity() override;
	unsigned int Move(const Vector3 &disp,float elapsedTime,float minDist=0.1f);
	ControllerHitData &GetControllerHitData();

	bool IsOnGround() const;
	bool IsGroundWalkable() const;
	const PhysContactInfo *GetGroundContactInfo() const;
	double GetMinGroundXZContactDistance() const;
	BaseEntity *GetGroundEntity() const;
	PhysObj *GetGroundPhysObject() const;
	int32_t GetGroundSurfaceMaterial() const;
	PhysCollisionObject *GetGroundPhysCollisionObject() const;
	// The velocity affecting this controller originating from the ground object
	Vector3 GetGroundVelocity() const;

	Vector3 GetDimensions() const;
	void SetDimensions(const Vector3 &dimensions);
	// Sets the friction for the next simulation step. The friction will be reset after the simulation step is complete.
	void SetCurrentFriction(Float friction);
	Float GetCurrentFriction() const;

	// These are called by the simulation; Don't call these manually!
	bool SetGroundContactPoint(const btManifoldPoint &contactPoint,int32_t idx,const btCollisionObject *o,const btCollisionObject *oOther);
	void ClearGroundContactPoint();
};

class DLLNETWORK BoxControllerPhysObj
	: public ControllerPhysObj
{
protected:
#if PHYS_ENGINE_PHYSX
	physx::PxBoxController *m_boxController;
#endif
	Vector3 m_halfExtents = {};
public:
#ifdef PHYS_ENGINE_BULLET
	BoxControllerPhysObj(pragma::BaseEntityComponent *owner,const Vector3 &halfExtents,unsigned int stepHeight);
#elif PHYS_ENGINE_PHYSX
	BoxControllerPhysObj(BaseEntity *owner,physx::PxBoxController *controller);
	physx::PxBoxController *GetController();
#endif
	Vector3 &GetHalfExtents();
	void SetCollisionBounds(const Vector3 &min,const Vector3 &max);
	void GetCollisionBounds(Vector3 *min,Vector3 *max);
	virtual void SetPosition(const Vector3 &pos) override;
	virtual Vector3 GetPosition() override;
};

class DLLNETWORK CapsuleControllerPhysObj
	: public ControllerPhysObj
{
protected:
#ifdef PHYS_ENGINE_BULLET
	float m_width = 0.f;
	float m_height = 0.f;
#elif PHYS_ENGINE_PHYSX
	physx::PxCapsuleController *m_capsuleController;
#endif
public:
#ifdef PHYS_ENGINE_BULLET
	CapsuleControllerPhysObj(pragma::BaseEntityComponent *owner,unsigned int width,unsigned int height,unsigned int stepHeight);
#elif PHYS_ENGINE_PHYSX
	CapsuleControllerPhysObj(BaseEntity *owner,physx::PxCapsuleController *controller);
	physx::PxCapsuleController *GetController();
#endif
	float GetWidth() const;
	float GetHeight() const;
	void SetHeight(float height);
	virtual bool IsCapsule() override;
	void SetCollisionBounds(const Vector3 &min,const Vector3 &max);
	void GetCollisionBounds(Vector3 *min,Vector3 *max);
	virtual void SetPosition(const Vector3 &pos) override;
	virtual Vector3 GetPosition() override;
};

////////////////////////////////////

#ifdef PHYS_ENGINE_PHYSX
class DLLNETWORK DynamicActorInfo
{
protected:
	physx::PxRigidDynamic *m_actor;
	int m_bone;
public:
	DynamicActorInfo(physx::PxRigidDynamic *actor,int bone=0);
	physx::PxRigidDynamic *GetActor();
	int GetBoneID();
};

class DLLNETWORK DynamicPhysObj
	: public PhysObj,public PhysObjKinematic,public PhysObjDynamic
{
private:
	std::vector<DynamicActorInfo> m_actorInfos;
	virtual void AddActor(physx::PxRigidActor *actor) override;
public:
	DynamicPhysObj(BaseEntity *owner,physx::PxRigidDynamic *actor,int bone=0);
	DynamicPhysObj(BaseEntity *owner,std::vector<physx::PxRigidActor*> *actors);
	unsigned int AddActor(physx::PxRigidDynamic *actor,int bone);
	unsigned int AddActor(physx::PxRigidDynamic *actor);
	int GetBoneID(physx::PxRigidDynamic *actor);
	virtual ~DynamicPhysObj() override;

	virtual void SetKinematic(bool b) override;
	DynamicActorInfo *GetActorInfo(unsigned int idx);
	std::vector<DynamicActorInfo> &GetActorInfo();
	void PutToSleep();
	void WakeUp();
	void Simulate(double tDelta,bool bIgnoreGravity=false);
	BaseEntity *GetOwner();

	Vector3 GetLinearVelocity();
	void SetLinearVelocity(const Vector3 &vel);
	void AddLinearVelocity(const Vector3 &vel);
	Vector3 GetAngularVelocity();
	void SetAngularVelocity(const Vector3 &vel);
	void AddAngularVelocity(const Vector3 &vel);
};

class DLLNETWORK StaticPhysObj
	: public PhysObj
{
private:
	std::vector<physx::PxRigidStatic*> m_staticActors;
public:
	StaticPhysObj(BaseEntity *owner,physx::PxRigidStatic *actor);
	StaticPhysObj(BaseEntity *owner,std::vector<physx::PxRigidActor*> *actors);
	StaticPhysObj(BaseEntity *owner,btCollisionObject *object);
	StaticPhysObj(BaseEntity *owner,std::vector<btCollisionObject*> *objects);
	bool IsStatic();
	void Enable();
	void Disable();
};
#endif

#endif
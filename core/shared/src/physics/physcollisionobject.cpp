#include "stdafx_shared.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/physshape.h"
#include "pragma/physics/physmotionstate.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/phys_kinematic_motion_state.hpp"

const double EPSILON = 4.9406564584125e-2;

DEFINE_BASE_HANDLE(DLLNETWORK,PhysCollisionObject,PhysCollisionObject);
DEFINE_DERIVED_HANDLE(DLLNETWORK,PhysCollisionObject,PhysCollisionObject,PhysRigidBody,PhysRigidBody);
DEFINE_DERIVED_HANDLE(DLLNETWORK,PhysCollisionObject,PhysCollisionObject,PhysSoftBody,PhysSoftBody);

class SimpleMotionState
	: public btMotionState
{
public:
	SimpleMotionState()=default;
	// TODO: Implement this
	virtual void getWorldTransform(btTransform &worldTrans) const override {}
	virtual void setWorldTransform(const btTransform &worldTrans) override {}
};

PhysCollisionObject::PhysCollisionObject(PhysEnv *env,btCollisionObject *o,std::shared_ptr<PhysShape> &shape)
	: PhysBase(env),LuaObj<PhysCollisionObjectHandle>(),m_collisionObject(o),m_shape(shape)
{
	o->setUserPointer(this);
	UpdateCCD();
	UpdateSurfaceMaterial();
}

PhysCollisionObject::~PhysCollisionObject()
{
	m_physEnv->RemoveCollisionObject(this);
}

void PhysCollisionObject::SetSimulationEnabled(bool b)
{
	if(b == IsSimulationEnabled())
		return;
	m_simulationEnabled.first = b;
	if(b == false)
	{
		m_collisionObject->setActivationState(DISABLE_SIMULATION);
		m_collisionObject->setCollisionFlags(m_collisionObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		m_simulationEnabled.second = GetCollisionFilterGroup();
		SetCollisionFilterGroup(CollisionMask::NoCollision);
	}
	else
	{
		m_collisionObject->setCollisionFlags(m_collisionObject->getCollisionFlags() &~btCollisionObject::CF_NO_CONTACT_RESPONSE);
		SetCollisionFilterGroup(m_simulationEnabled.second);
		m_collisionObject->forceActivationState(ACTIVE_TAG);
		m_collisionObject->activate();
	}
}
void PhysCollisionObject::DisableSimulation() {SetSimulationEnabled(false);}
void PhysCollisionObject::EnableSimulation() {SetSimulationEnabled(true);}
bool PhysCollisionObject::IsSimulationEnabled() const {return m_simulationEnabled.first;}

void PhysCollisionObject::SetCCDEnabled(bool b)
{
	m_bCcdEnabled = b;
	UpdateCCD();
}

void PhysCollisionObject::UpdateCCD()
{
	if(m_collisionObject == nullptr)
		return;
	// Dirty hack: CCD does not work with custom collision rules, so disable it if there are any
	if(m_bCcdEnabled == false || m_collisionFilterGroup != CollisionMask::Default || m_collisionFilterMask != CollisionMask::Default)
	{
		m_collisionObject->setCcdMotionThreshold(0.0);
		m_collisionObject->setCcdSweptSphereRadius(0.0);
	}
	else
	{
		m_collisionObject->setCcdMotionThreshold(PhysEnv::CCD_MOTION_THRESHOLD);
		m_collisionObject->setCcdSweptSphereRadius(PhysEnv::CCD_SWEPT_SPHERE_RADIUS);
	}
}

UInt32 PhysCollisionObject::GetBoneID() const {return m_boneId;}
void PhysCollisionObject::SetBoneID(UInt32 id) {m_boneId = id;}

void PhysCollisionObject::GetAABB(Vector3 &min,Vector3 &max) const
{
	if(m_shape == nullptr)
	{
		min = {};
		max = {};
		return;
	}
	m_shape->GetAABB(min,max);
}

void PhysCollisionObject::UpdateSurfaceMaterial()
{
	if(m_customSurfaceMaterial == true)
		return;
	auto *shape = m_shape.get();
	auto *hullShape = dynamic_cast<PhysConvexHullShape*>(shape);
	if(hullShape == nullptr)
		return;
	m_surfaceMaterial = hullShape->GetSurfaceMaterial();
}

int PhysCollisionObject::GetSurfaceMaterial() const {return m_surfaceMaterial;}
void PhysCollisionObject::SetSurfaceMaterial(int id) {m_surfaceMaterial = id; m_customSurfaceMaterial = true;}
void PhysCollisionObject::InitializeHandle() {m_handle = new PhysCollisionObjectHandle(this);}

bool PhysCollisionObject::IsRigid() const {return false;}
bool PhysCollisionObject::IsGhost() const {return false;}
bool PhysCollisionObject::IsSoftBody() const {return false;}

void PhysCollisionObject::Initialize()
{
	InitializeHandle();
	auto *nwState = m_physEnv->GetNetworkState();
	InitializeLuaObject(nwState->GetLuaState());
}

void PhysCollisionObject::Spawn()
{
	if(m_bSpawned == true)
		return;
	m_bSpawned = true;
	SetCollisionShape(m_shape); // Add it to the physics environment
	auto *broadphase = m_collisionObject->getBroadphaseHandle();
	if(broadphase == nullptr)
		return;
	broadphase->m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(m_collisionFilterMask));
	broadphase->m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(m_collisionFilterGroup));
	UpdateCCD();
}

void PhysCollisionObject::SetTrigger(bool)
{
	m_collisionObject->setCollisionFlags(m_collisionObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
}
bool PhysCollisionObject::IsTrigger()
{
	int flags = m_collisionObject->getCollisionFlags();
	return (flags &btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0 ? true : false;
}

PhysTransform PhysCollisionObject::GetWorldTransform() {return PhysTransform(m_collisionObject->getWorldTransform());}

void PhysCollisionObject::SetWorldTransform(const PhysTransform &t)
{
	const btTransform &bt = t.GetTransform();
	m_collisionObject->setWorldTransform(bt);
}

btCollisionObject *PhysCollisionObject::GetCollisionObject() {return m_collisionObject;}

Bool PhysCollisionObject::HasOrigin() const {return m_bHasOrigin;}
void PhysCollisionObject::SetOrigin(const Vector3 &origin)
{
	m_origin = origin;
	if(
		m_origin.x < EPSILON && m_origin.x > -EPSILON &&
		m_origin.y < EPSILON && m_origin.y > -EPSILON &&
		m_origin.z < EPSILON && m_origin.z > -EPSILON
	)
	{
		m_origin = Vector3(0.f,0.f,0.f);
		m_bHasOrigin = false;
	}
	else
		m_bHasOrigin = true;
}
Vector3 &PhysCollisionObject::GetOrigin() {return m_origin;}

Vector3 PhysCollisionObject::GetPos() const
{
	btTransform &t = m_collisionObject->getWorldTransform();
	btVector3 &p = t.getOrigin();
	return Vector3(p.x() /PhysEnv::WORLD_SCALE,p.y() /PhysEnv::WORLD_SCALE,p.z() /PhysEnv::WORLD_SCALE);
}

void PhysCollisionObject::SetPos(const Vector3 &pos)
{
	btTransform &t = m_collisionObject->getWorldTransform();
	btVector3 &p = t.getOrigin();
	p.setX(pos.x *PhysEnv::WORLD_SCALE);
	p.setY(pos.y *PhysEnv::WORLD_SCALE);
	p.setZ(pos.z *PhysEnv::WORLD_SCALE);

	UpdateAABB();
}

Quat PhysCollisionObject::GetRotation() const
{
	btTransform &t = m_collisionObject->getWorldTransform();
	btQuaternion rot = t.getRotation();
	return Quat(
		static_cast<float>(rot.w()),
		static_cast<float>(rot.x()),
		static_cast<float>(rot.y()),
		static_cast<float>(rot.z())
	);
}

void PhysCollisionObject::SetRotation(const Quat &rot)
{
	btTransform &t = m_collisionObject->getWorldTransform();
	t.setRotation(btQuaternion(rot.x,rot.y,rot.z,rot.w));
}

int PhysCollisionObject::GetCollisionFlags() const {return m_collisionObject->getCollisionFlags();}

void PhysCollisionObject::SetCollisionFlags(int flags) {return m_collisionObject->setCollisionFlags(flags);}

void PhysCollisionObject::Activate(bool forceActivation) {m_collisionObject->activate(forceActivation);}

int PhysCollisionObject::GetActivationState() {return m_collisionObject->getActivationState();}
void PhysCollisionObject::SetActivationState(int state) {m_collisionObject->setActivationState(state);}

void PhysCollisionObject::AddWorldObject()
{
	if(m_bSpawned == false)
		return;
	RemoveWorldObject();
	auto *world = m_physEnv->GetWorld();
	world->addCollisionObject(m_collisionObject,umath::to_integral(m_collisionFilterGroup),umath::to_integral(m_collisionFilterMask));
}

void PhysCollisionObject::RemoveWorldObject()
{
	auto *world = m_physEnv->GetWorld();
	world->removeCollisionObject(m_collisionObject);
}

void PhysCollisionObject::UpdateAABB() {m_bUpdateAABB = true;}
bool PhysCollisionObject::ShouldUpdateAABB() const {return m_bUpdateAABB;}
void PhysCollisionObject::ResetUpdateAABBFlag() {m_bUpdateAABB = false;}

void PhysCollisionObject::SetCollisionShape(std::shared_ptr<PhysShape> &shape)
{
	m_shape = shape;
	if(shape != nullptr)
	{
		btCollisionShape *btShape = shape->GetShape();
		m_collisionObject->setCollisionShape(btShape);
		UpdateSurfaceMaterial();
	}
	else
		m_collisionObject->setCollisionShape(nullptr);
	AddWorldObject();
}

std::shared_ptr<PhysShape> PhysCollisionObject::GetCollisionShape() const {return m_shape;}

void PhysCollisionObject::SetContactProcessingThreshold(float threshold)
{
	m_collisionObject->setContactProcessingThreshold(threshold);
}

void PhysCollisionObject::SetCollisionFilterGroup(CollisionMask group)
{
	m_collisionFilterGroup = group;
	if(m_collisionObject == nullptr)
		return;
	auto *broadphase = m_collisionObject->getBroadphaseHandle();
	if(broadphase == nullptr)
		return;
	broadphase->m_collisionFilterGroup = static_cast<int16_t>(umath::to_integral(group));
}
CollisionMask PhysCollisionObject::GetCollisionFilterGroup() const {return m_collisionFilterGroup;}
void PhysCollisionObject::SetCollisionFilterMask(CollisionMask mask)
{
	m_collisionFilterMask = mask;
	if(m_collisionObject == nullptr)
		return;
	auto *broadphase = m_collisionObject->getBroadphaseHandle();
	if(broadphase == nullptr)
		return;
	broadphase->m_collisionFilterMask = static_cast<int16_t>(umath::to_integral(mask));
}
CollisionMask PhysCollisionObject::GetCollisionFilterMask() const {return m_collisionFilterMask;}

void PhysCollisionObject::PreSimulate() {}
void PhysCollisionObject::PostSimulate() {}

//////////////////////////

PhysRigidBody::PhysRigidBody(PhysEnv *env,btRigidBody *body,float mass,std::shared_ptr<PhysShape> &shape,const Vector3 &localInertia)
	: PhysCollisionObject(env,body,shape),m_rigidBody(body)
{
	SetMassProps(mass,localInertia);
	m_motionState = std::make_unique<SimpleMotionState>();
	body->setMotionState(m_motionState.get());
}

PhysRigidBody::PhysRigidBody(PhysEnv *env,float mass,std::shared_ptr<PhysShape> &shape,const Vector3 &localInertia)
	: PhysRigidBody(env,new btRigidBody(mass,nullptr,shape->GetShape(),btVector3(localInertia.x,localInertia.y,localInertia.z) *PhysEnv::WORLD_SCALE),mass,shape,localInertia)
{}

PhysRigidBody::~PhysRigidBody()
{}

void PhysRigidBody::SetDamping(float linDamping,float angDamping)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->setDamping(linDamping,angDamping);
}
void PhysRigidBody::SetLinearDamping(float damping)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->setDamping(damping,body->getAngularDamping());
}
void PhysRigidBody::SetAngularDamping(float damping)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->setDamping(body->getLinearDamping(),damping);
}
float PhysRigidBody::GetLinearDamping() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return 0.f;
	return CFloat(body->getLinearDamping());
}
float PhysRigidBody::GetAngularDamping() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return 0.f;
	return CFloat(body->getAngularDamping());
}
void PhysRigidBody::SetLinearSleepingThreshold(float threshold) {SetSleepingThresholds(threshold,GetAngularSleepingThreshold());}
void PhysRigidBody::SetAngularSleepingThreshold(float threshold) {SetSleepingThresholds(GetLinearSleepingThreshold(),threshold);}
void PhysRigidBody::SetSleepingThresholds(float linear,float angular)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->setSleepingThresholds(linear,angular);
}
float PhysRigidBody::GetLinearSleepingThreshold() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return 0.f;
	return body->getLinearSleepingThreshold();
}
float PhysRigidBody::GetAngularSleepingThreshold() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return 0.f;
	return body->getAngularSleepingThreshold();
}
std::pair<float,float> PhysRigidBody::GetSleepingThreshold() const {return {GetLinearSleepingThreshold(),GetAngularSleepingThreshold()};}

void PhysRigidBody::ClearForces()
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	return body->clearForces();
}
Vector3 PhysRigidBody::GetTotalForce()
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto force = body->getTotalForce() /PhysEnv::WORLD_SCALE;
	return Vector3(force.x(),force.y(),force.z());
}
Vector3 PhysRigidBody::GetTotalTorque()
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto force = body->getTotalTorque() /PhysEnv::WORLD_SCALE_SQR;
	return Vector3(force.x(),force.y(),force.z());
}

void PhysRigidBody::InitializeHandle() {m_handle = new PhysRigidBodyHandle(this);}

void PhysRigidBody::InitializeLuaObject(lua_State *lua) {PhysCollisionObject::InitializeLuaObject<PhysRigidBodyHandle>(lua);}

btRigidBody *PhysRigidBody::GetRigidBody() const {return m_rigidBody;}

bool PhysRigidBody::IsRigid() const {return true;}

void PhysRigidBody::AddWorldObject()
{
	if(m_bSpawned == false)
		return;
	RemoveWorldObject();
	auto *world = m_physEnv->GetWorld();
	world->addRigidBody(m_rigidBody,umath::to_integral(m_collisionFilterGroup),umath::to_integral(m_collisionFilterMask));
}

void PhysRigidBody::RemoveWorldObject()
{
	auto *world = m_physEnv->GetWorld();
	world->removeRigidBody(m_rigidBody);
}

bool PhysRigidBody::IsStatic() const {return GetMass() == 0.f && IsKinematic() == false;}

void PhysRigidBody::ApplyForce(const Vector3 &force)
{
	if(IsStatic())
		return;
	m_rigidBody->activate(true);
	m_rigidBody->applyCentralForce(btVector3(force.x,force.y,force.z) *PhysEnv::WORLD_SCALE *m_physEnv->GetTimeScale());
}
void PhysRigidBody::ApplyForce(const Vector3 &force,const Vector3 &relPos)
{
	if(IsStatic())
		return;
	m_rigidBody->activate(true);
	auto btRelPos = uvec::create_bt(relPos) *PhysEnv::WORLD_SCALE -m_rigidBody->getCenterOfMassPosition();
	m_rigidBody->applyForce(btVector3(force.x,force.y,force.z) *PhysEnv::WORLD_SCALE *m_physEnv->GetTimeScale(),btRelPos);
}
void PhysRigidBody::ApplyImpulse(const Vector3 &impulse)
{
	if(IsStatic())
		return;
	m_rigidBody->activate(true);
	m_rigidBody->applyCentralImpulse(btVector3(impulse.x,impulse.y,impulse.z) *PhysEnv::WORLD_SCALE);
}
void PhysRigidBody::ApplyImpulse(const Vector3 &impulse,const Vector3 &relPos)
{
	if(IsStatic())
		return;
	m_rigidBody->activate(true);
	auto btRelPos = uvec::create_bt(relPos) *PhysEnv::WORLD_SCALE -m_rigidBody->getCenterOfMassPosition();
	m_rigidBody->applyImpulse(btVector3(impulse.x,impulse.y,impulse.z) *PhysEnv::WORLD_SCALE,btRelPos);
}
void PhysRigidBody::ApplyTorque(const Vector3 &torque)
{
	if(IsStatic())
		return;
	m_rigidBody->activate(true);
	auto ts = m_physEnv->GetTimeScale();
	m_rigidBody->applyTorque(btVector3(torque.x,torque.y,torque.z) *PhysEnv::WORLD_SCALE_SQR *(ts *ts));
}
void PhysRigidBody::ApplyTorqueImpulse(const Vector3 &torque)
{
	if(IsStatic())
		return;
	m_rigidBody->activate(true);
	m_rigidBody->applyTorqueImpulse(btVector3(torque.x,torque.y,torque.z) *PhysEnv::WORLD_SCALE_SQR);
}

void PhysRigidBody::SetMassProps(float mass,const Vector3 &inertia)
{
	m_rigidBody->setMassProps(mass,btVector3(inertia.x,inertia.y,inertia.z) *PhysEnv::WORLD_SCALE);
	m_mass = mass;
	m_inertia = inertia;
	SetCollisionFlags(GetCollisionFlags() &~btCollisionObject::CF_STATIC_OBJECT);

	// BUG: If CF_STATIC_OBJECT is set, soft-body objects will not collide with static rigid bodies! (Cause unknown)
	if(mass != 0.f)
		SetCollisionFlags(GetCollisionFlags() &~btCollisionObject::CF_STATIC_OBJECT);
	else
		SetCollisionFlags(GetCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
}

void PhysRigidBody::SetKinematic(bool bKinematic)
{
	if(bKinematic == IsKinematic())
		return;
	auto flags = GetCollisionFlags();
	if(bKinematic)
		flags |= btCollisionObject::CF_KINEMATIC_OBJECT;
	else
		flags &= ~btCollisionObject::CF_KINEMATIC_OBJECT;
	SetCollisionFlags(flags);

	if(bKinematic == false)
		m_motionState = std::make_unique<SimpleMotionState>();
	else
		m_motionState = std::make_unique<KinematicMotionState>(PhysTransform{m_rigidBody->getWorldTransform()});
	m_rigidBody->setMotionState(m_motionState.get());

	m_kinematicData = {};
}
bool PhysRigidBody::IsKinematic() const
{
	return (GetCollisionFlags() &btCollisionObject::CF_KINEMATIC_OBJECT) != 0;
}

int PhysRigidBody::GetFlags() {return m_rigidBody->getFlags();}

void PhysRigidBody::SetFlags(int flags) {return m_rigidBody->setFlags(flags);}

Vector3 PhysRigidBody::GetLinearVelocity()
{
	if(IsStatic() == true)
		return {};
	if(IsKinematic())
		return m_kinematicData.linearVelocity;
	const btVector3 &linVel = m_rigidBody->getLinearVelocity();
	return Vector3(linVel.x() /PhysEnv::WORLD_SCALE,linVel.y() /PhysEnv::WORLD_SCALE,linVel.z() /PhysEnv::WORLD_SCALE);
}
Vector3 PhysRigidBody::GetAngularVelocity()
{
	if(IsStatic() == true)
		return {};
	if(IsKinematic())
		return m_kinematicData.angularVelocity;
	const btVector3 &angVel = m_rigidBody->getAngularVelocity();
	return Vector3(angVel.x(),angVel.y(),angVel.z());
}

void PhysRigidBody::SetLinearVelocity(const Vector3 &vel)
{
	if(IsStatic() == true)
	{
		m_rigidBody->setLinearVelocity(btVector3{0.f,0.f,0.f}); // Can't apply velocity to static objects
		return;
	}
	m_rigidBody->activate(true);
	m_rigidBody->setLinearVelocity(btVector3(vel.x,vel.y,vel.z) *PhysEnv::WORLD_SCALE);
	if(IsKinematic())
		m_kinematicData.linearVelocity = vel;
}
void PhysRigidBody::SetAngularVelocity(const Vector3 &vel)
{
	if(IsStatic() == true)
	{
		m_rigidBody->setAngularVelocity(btVector3{0.f,0.f,0.f}); // Can't apply angular velocity to static objects
		return;
	}
	m_rigidBody->activate(true);
	m_rigidBody->setAngularVelocity(btVector3(vel.x,vel.y,vel.z));
	if(IsKinematic())
		m_kinematicData.angularVelocity = vel;
}
float PhysRigidBody::GetMass() const {return m_mass;}
void PhysRigidBody::SetMass(float mass)
{
	Vector3 localInertia(0.f,0.f,0.f);
	m_shape->CalculateLocalInertia(mass,&localInertia);
	SetMassProps(mass,localInertia);
	if(mass == 0.f)
	{
		SetLinearVelocity({});
		SetAngularVelocity({});
	}
}
Vector3 &PhysRigidBody::GetInertia() {return m_inertia;}
Mat3 PhysRigidBody::GetInvInertiaTensorWorld() const
{
	m_rigidBody->updateInertiaTensor();
	auto &inertia = m_rigidBody->getInvInertiaTensorWorld();
	return Mat3{
		inertia[0][0],inertia[0][1],inertia[0][2],
		inertia[1][0],inertia[1][1],inertia[1][2],
		inertia[2][0],inertia[2][1],inertia[2][2],
	} *static_cast<float>(PhysEnv::WORLD_SCALE_SQR);
}
void PhysRigidBody::SetInertia(const Vector3 &inertia) {SetMassProps(m_mass,inertia);}

void PhysRigidBody::SetLinearFactor(const Vector3 &factor) {m_rigidBody->setLinearFactor(btVector3(factor.x,factor.y,factor.z));}
void PhysRigidBody::SetAngularFactor(const Vector3 &factor) {m_rigidBody->setAngularFactor(btVector3(factor.x,factor.y,factor.z));}
Vector3 PhysRigidBody::GetLinearFactor() const
{
	auto &factor = m_rigidBody->getLinearFactor();
	return Vector3(factor.x(),factor.y(),factor.z());
}
Vector3 PhysRigidBody::GetAngularFactor() const
{
	auto &factor = m_rigidBody->getAngularFactor();
	return Vector3(factor.x(),factor.y(),factor.z());
}
Vector3 PhysRigidBody::GetPos() const
{
	if(IsKinematic())
		return static_cast<KinematicMotionState&>(*m_motionState).GetWorldTransform().GetOrigin();
	return PhysCollisionObject::GetPos();
}
void PhysRigidBody::SetPos(const Vector3 &pos)
{
	PhysCollisionObject::SetPos(pos);
	if(IsKinematic())
	{
		auto &t = static_cast<KinematicMotionState&>(*m_motionState).GetWorldTransform();
		t.SetOrigin(pos);
	}
}
Quat PhysRigidBody::GetRotation() const
{
	if(IsKinematic())
		return static_cast<KinematicMotionState&>(*m_motionState).GetWorldTransform().GetRotation();
	return PhysCollisionObject::GetRotation();
}
void PhysRigidBody::SetRotation(const Quat &rot)
{
	PhysCollisionObject::SetRotation(rot);
	if(IsKinematic())
	{
		auto &t = static_cast<KinematicMotionState&>(*m_motionState).GetWorldTransform();
		t.SetRotation(rot);
	}
}
void PhysRigidBody::PreSimulate()
{
	auto *nw = m_physEnv->GetNetworkState();
	auto *game = nw->GetGameState();
	auto dt = game->DeltaTickTime();
	if(dt > 0.0)
		SetLinearVelocity(GetLinearVelocity() +GetLinearCorrectionVelocity() /static_cast<float>(dt));
	if(IsKinematic())
	{
		auto &transform = static_cast<KinematicMotionState*>(m_motionState.get())->GetWorldTransform();
		transform.SetOrigin(transform.GetOrigin() +GetLinearVelocity() *static_cast<float>(dt));
		auto angVel = GetAngularVelocity() *static_cast<float>(dt);
		EulerAngles ang {
			static_cast<float>(umath::rad_to_deg(angVel.x)),
			static_cast<float>(umath::rad_to_deg(angVel.y)),
			static_cast<float>(umath::rad_to_deg(angVel.z))
		};
		transform.SetRotation(transform.GetRotation() *uquat::create(ang));
	}
	PhysCollisionObject::PreSimulate();
}
void PhysRigidBody::PostSimulate()
{
	auto *nw = m_physEnv->GetNetworkState();
	auto *game = nw->GetGameState();
	auto dt = game->DeltaTickTime();
	if(dt > 0.0)
		SetLinearVelocity(GetLinearVelocity() -GetLinearCorrectionVelocity() /static_cast<float>(dt));
	ResetLinearCorrectionVelocity();
	PhysCollisionObject::PostSimulate();
}

//////////////////////////

PhysSoftBody::PhysSoftBody(PhysEnv *env,btSoftBody *o,std::shared_ptr<PhysShape> &shape,const std::vector<uint16_t> &meshVertIndicesToPhysIndices)
	: PhysCollisionObject(env,o,shape),m_softBody(o),
	m_rotation{},m_localVertexIndicesToNodeIndices(meshVertIndicesToPhysIndices)
{
	m_nodeIndicesToLocalVertexIndices.resize(meshVertIndicesToPhysIndices.size());
	for(auto i=decltype(meshVertIndicesToPhysIndices.size()){0};i<meshVertIndicesToPhysIndices.size();++i)
		m_nodeIndicesToLocalVertexIndices.at(meshVertIndicesToPhysIndices.at(i)) = i;
	UpdateTotalMass();
	UpdateLinearVelocity();
}

const std::vector<uint16_t> &PhysSoftBody::GetMeshVertexIndicesToLocalIndices() const {return m_meshVertexIndicesToLocalVertexIndices;}
const std::vector<uint16_t> &PhysSoftBody::GetLocalVertexIndicesToNodeIndices() const {return m_localVertexIndicesToNodeIndices;}
const std::vector<uint16_t> &PhysSoftBody::GetLocalVertexIndicesToMeshVertexIndices() const {return m_localVertexIndicesToMeshVertexIndices;}
const std::vector<uint16_t> &PhysSoftBody::GetNodeIndicesToLocalVertexIndices() const {return m_nodeIndicesToLocalVertexIndices;}

bool PhysSoftBody::MeshVertexIndexToLocalVertexIndex(uint16_t meshVertexIndex,uint16_t &localIndex) const
{
	if(meshVertexIndex >= m_meshVertexIndicesToLocalVertexIndices.size())
		return false;
	localIndex = m_meshVertexIndicesToLocalVertexIndices.at(meshVertexIndex);
	return true;
}
bool PhysSoftBody::LocalVertexIndexToMeshVertexIndex(uint16_t localIndex,uint16_t &meshVertexIndex) const
{
	if(localIndex >= m_localVertexIndicesToMeshVertexIndices.size())
		return false;
	meshVertexIndex = m_localVertexIndicesToMeshVertexIndices.at(localIndex);
	return true;
}
bool PhysSoftBody::LocalVertexIndexToNodeIndex(uint16_t localVertexIndex,uint16_t &nodeIndex) const
{
	if(localVertexIndex >= m_localVertexIndicesToNodeIndices.size())
		return false;
	nodeIndex = m_localVertexIndicesToNodeIndices.at(localVertexIndex);
	return true;
}
bool PhysSoftBody::NodeIndexToLocalVertexIndex(uint16_t nodeIndex,uint16_t &localVertexIndex) const
{
	if(nodeIndex >= m_nodeIndicesToLocalVertexIndices.size())
		return false;
	localVertexIndex = m_nodeIndicesToLocalVertexIndices.at(nodeIndex);
	return true;
}
	
bool PhysSoftBody::MeshVertexIndexToNodeIndex(uint16_t meshVertexIndex,uint16_t &nodeIndex) const
{
	uint16_t localIndex = 0u;
	if(MeshVertexIndexToLocalVertexIndex(meshVertexIndex,localIndex) == false)
		return false;
	return LocalVertexIndexToNodeIndex(localIndex,nodeIndex);
}
bool PhysSoftBody::NodeIndexToMeshVertexIndex(uint16_t nodeIndex,uint16_t &meshVertexIndex) const
{
	uint16_t localIndex = 0u;
	if(NodeIndexToLocalVertexIndex(nodeIndex,localIndex) == false)
		return false;
	return LocalVertexIndexToMeshVertexIndex(localIndex,meshVertexIndex);
}
void PhysSoftBody::SetSubMesh(const ModelSubMesh &subMesh,const std::vector<uint16_t> &meshVertexIndicesToLocalVertexIndices)
{
	m_subMesh = const_cast<ModelSubMesh&>(subMesh).shared_from_this();
	m_meshVertexIndicesToLocalVertexIndices = meshVertexIndicesToLocalVertexIndices;

	m_localVertexIndicesToMeshVertexIndices.clear();
	m_localVertexIndicesToMeshVertexIndices.resize(GetNodeCount(),std::numeric_limits<uint16_t>::max());
	for(auto i=decltype(m_meshVertexIndicesToLocalVertexIndices.size()){0};i<m_meshVertexIndicesToLocalVertexIndices.size();++i)
	{
		auto localIdx = meshVertexIndicesToLocalVertexIndices.at(i);
		if(localIdx == std::numeric_limits<uint16_t>::max())
			continue;
		if(localIdx >= m_localVertexIndicesToMeshVertexIndices.size())
		{
			Con::cwar<<"WARNING: Invalid soft-body node index "<<localIdx<<"! Skipping..."<<Con::endl;
			continue;
		}
		m_localVertexIndicesToMeshVertexIndices.at(localIdx) = i;
	}
}
ModelSubMesh *PhysSoftBody::GetSubMesh() const {return m_subMesh.expired() ? nullptr : m_subMesh.lock().get();}

void PhysSoftBody::AppendAnchor(uint32_t nodeId,PhysRigidBody &body,const Vector3 &localPivot,bool bDisableCollision,float influence)
{
	assert(nodeId < m_softBody->m_nodes.size());
	if(nodeId >= m_softBody->m_nodes.size())
	{
		Con::cwar<<"WARNING: Attempted to add soft-body anchor for invalid node "<<nodeId<<"! Skipping..."<<Con::endl;
		return;
	}
	m_softBody->appendAnchor(nodeId,body.GetRigidBody(),uvec::create_bt(localPivot) *PhysEnv::WORLD_SCALE,bDisableCollision,influence);
}
void PhysSoftBody::AppendAnchor(uint32_t nodeId,PhysRigidBody &body,bool bDisableCollision,float influence)
{
	assert(nodeId < m_softBody->m_nodes.size());
	if(nodeId >= m_softBody->m_nodes.size())
	{
		Con::cwar<<"WARNING: Attempted to add soft-body anchor for invalid node "<<nodeId<<"! Skipping..."<<Con::endl;
		return;
	}
	m_softBody->appendAnchor(nodeId,body.GetRigidBody(),bDisableCollision,influence);
}
const btAlignedObjectArray<btSoftBody::Node> &PhysSoftBody::GetNodes() const {return m_softBody->m_nodes;}
uint32_t PhysSoftBody::GetNodeCount() const {return m_softBody->m_nodes.size();}

void PhysSoftBody::InitializeHandle() {m_handle = new PhysSoftBodyHandle(this);}

void PhysSoftBody::InitializeLuaObject(lua_State *lua) {PhysCollisionObject::InitializeLuaObject<PhysSoftBodyHandle>(lua);}

void PhysSoftBody::GetAABB(Vector3 &min,Vector3 &max) const
{
	btVector3 btMin,btMax;
	m_softBody->getAabb(btMin,btMax);
	min.x = static_cast<float>(btMin.x());
	min.y = static_cast<float>(btMin.y());
	min.z = static_cast<float>(btMin.z());

	max.x = static_cast<float>(btMax.x());
	max.y = static_cast<float>(btMax.y());
	max.z = static_cast<float>(btMax.z());
	min /= PhysEnv::WORLD_SCALE;
	max /= PhysEnv::WORLD_SCALE;
}
void PhysSoftBody::AddAeroForceToNode(int32_t node,const Vector3 &force) {m_softBody->addAeroForceToNode(uvec::create_bt(force) *PhysEnv::WORLD_SCALE,node);}
void PhysSoftBody::AddAeroForceToFace(int32_t face,const Vector3 &force) {m_softBody->addAeroForceToFace(uvec::create_bt(force) *PhysEnv::WORLD_SCALE,face);}
void PhysSoftBody::AddForce(const Vector3 &force) {m_softBody->addForce(uvec::create_bt(force) *PhysEnv::WORLD_SCALE);}
void PhysSoftBody::AddForce(uint32_t node,const Vector3 &force) {m_softBody->addForce(uvec::create_bt(force) *PhysEnv::WORLD_SCALE,node);}
void PhysSoftBody::AddLinearVelocity(const Vector3 &vel) {m_softBody->addVelocity(uvec::create_bt(vel) *PhysEnv::WORLD_SCALE);}
void PhysSoftBody::AddLinearVelocity(uint32_t node,const Vector3 &vel) {m_softBody->addVelocity(uvec::create_bt(vel) *PhysEnv::WORLD_SCALE,node);}
float PhysSoftBody::GetFriction() const {return static_cast<float>(m_softBody->getFriction());}
float PhysSoftBody::GetHitFraction() const {return static_cast<float>(m_softBody->getHitFraction());}
float PhysSoftBody::GetRollingFriction() const {return static_cast<float>(m_softBody->getRollingFriction());}
Vector3 PhysSoftBody::GetAnisotropicFriction() const {return uvec::create(m_softBody->getAnisotropicFriction());}
void PhysSoftBody::SetFriction(float friction) {m_softBody->setFriction(friction);}
void PhysSoftBody::SetHitFraction(float fraction) {m_softBody->setHitFraction(fraction);}
void PhysSoftBody::SetRollingFriction(float friction) {m_softBody->setRollingFriction(friction);}
void PhysSoftBody::SetAnisotropicFriction(const Vector3 &friction) {m_softBody->setAnisotropicFriction(uvec::create_bt(friction));}
float PhysSoftBody::GetMass(int32_t node) const {return static_cast<float>(m_softBody->getMass(node));}
float PhysSoftBody::GetMass() const {return m_totalMass;}
float PhysSoftBody::GetRestitution() const {return static_cast<float>(m_softBody->getRestitution());}
float PhysSoftBody::GetRestLengthScale() const {return static_cast<float>(m_softBody->getRestLengthScale());}
Vector3 PhysSoftBody::GetWindVelocity() const {return uvec::create(m_softBody->getWindVelocity() /PhysEnv::WORLD_SCALE);}
void PhysSoftBody::SetMass(int32_t node,float mass) {m_softBody->setMass(node,mass); UpdateTotalMass();}
void PhysSoftBody::SetMass(float mass) {m_softBody->setTotalMass(mass); UpdateTotalMass();}
void PhysSoftBody::SetRestitution(float rest) {m_softBody->setRestitution(rest);}
void PhysSoftBody::SetRestLengthScale(float scale) {m_softBody->setRestLengthScale(scale);}
void PhysSoftBody::SetWindVelocity(const Vector3 &vel) {m_softBody->setWindVelocity(uvec::create_bt(vel) *PhysEnv::WORLD_SCALE);}
void PhysSoftBody::SetLinearVelocity(const Vector3 &vel) {m_softBody->setVelocity(uvec::create_bt(vel) *PhysEnv::WORLD_SCALE);}
void PhysSoftBody::SetVolumeDensity(float density) {m_softBody->setVolumeDensity(density);}
void PhysSoftBody::SetVolumeMass(float mass) {m_softBody->setVolumeMass(mass);}
float PhysSoftBody::GetVolume() const {return static_cast<float>(m_softBody->getVolume());}
void PhysSoftBody::SetDensity(float density) {m_softBody->setTotalDensity(density);}
const Vector3 &PhysSoftBody::GetLinearVelocity() const {return m_linearVelocity;}
void PhysSoftBody::UpdateTotalMass() {m_totalMass = m_softBody->getTotalMass();}
void PhysSoftBody::UpdateLinearVelocity()
{
	auto linVel = btVector3{0.f,0.f,0.f};
	for(auto i=decltype(m_softBody->m_nodes.size()){0};i<m_softBody->m_nodes.size();++i)
	{
		auto &node = m_softBody->m_nodes.at(i);
		linVel += node.m_v;
	}
	linVel /= static_cast<float>(m_softBody->m_nodes.size());
	m_linearVelocity = uvec::create(linVel /PhysEnv::WORLD_SCALE);
}

void PhysSoftBody::SetAnchorsHardness(float val) {m_softBody->m_cfg.kAHR = val;}
void PhysSoftBody::SetRigidContactsHardness(float val) {m_softBody->m_cfg.kCHR = val;}
void PhysSoftBody::SetDynamicFrictionCoefficient(float val) {m_softBody->m_cfg.kDF = val;}
void PhysSoftBody::SetDragCoefficient(float val) {m_softBody->m_cfg.kDG = val;}
void PhysSoftBody::SetDampingCoefficient(float val) {m_softBody->m_cfg.kDP = val;}
void PhysSoftBody::SetKineticContactsHardness(float val) {m_softBody->m_cfg.kKHR = val;}
void PhysSoftBody::SetLiftCoefficient(float val) {m_softBody->m_cfg.kLF = val;}
void PhysSoftBody::SetPoseMatchingCoefficient(float val) {m_softBody->m_cfg.kMT = val;}
void PhysSoftBody::SetPressureCoefficient(float val) {m_softBody->m_cfg.kPR = val;}
void PhysSoftBody::SetSoftContactsHardness(float val) {m_softBody->m_cfg.kSHR = val;}
void PhysSoftBody::SetSoftVsKineticHardness(float val) {m_softBody->m_cfg.kSKHR_CL = val;}
void PhysSoftBody::SetSoftVsRigidImpulseSplitK(float val) {m_softBody->m_cfg.kSK_SPLT_CL = val;}
void PhysSoftBody::SetSoftVsRigidHardness(float val) {m_softBody->m_cfg.kSRHR_CL = val;}
void PhysSoftBody::SetSoftVsRigidImpulseSplitR(float val) {m_softBody->m_cfg.kSR_SPLT_CL = val;}
void PhysSoftBody::SetSoftVsSoftHardness(float val) {m_softBody->m_cfg.kSSHR_CL = val;}
void PhysSoftBody::SetSoftVsRigidImpulseSplitS(float val) {m_softBody->m_cfg.kSS_SPLT_CL = val;}
void PhysSoftBody::SetVolumeConversationCoefficient(float val) {m_softBody->m_cfg.kVC = val;}
void PhysSoftBody::SetVelocitiesCorrectionFactor(float val) {m_softBody->m_cfg.kVCF = val;}

float PhysSoftBody::GetAnchorsHardness() const {return m_softBody->m_cfg.kAHR;}
float PhysSoftBody::GetRigidContactsHardness() const {return m_softBody->m_cfg.kCHR;}
float PhysSoftBody::GetDynamicFrictionCoefficient() const {return m_softBody->m_cfg.kDF;}
float PhysSoftBody::GetDragCoefficient() const {return m_softBody->m_cfg.kDG;}
float PhysSoftBody::GetDampingCoefficient() const {return m_softBody->m_cfg.kDP;}
float PhysSoftBody::GetKineticContactsHardness() const {return m_softBody->m_cfg.kKHR;}
float PhysSoftBody::GetLiftCoefficient() const {return m_softBody->m_cfg.kLF;}
float PhysSoftBody::GetPoseMatchingCoefficient() const {return m_softBody->m_cfg.kMT;}
float PhysSoftBody::GetPressureCoefficient() const {return m_softBody->m_cfg.kPR;}
float PhysSoftBody::GetSoftContactsHardness() const {return m_softBody->m_cfg.kSHR;}
float PhysSoftBody::GetSoftVsKineticHardness() const {return m_softBody->m_cfg.kSKHR_CL;}
float PhysSoftBody::GetSoftVsRigidImpulseSplitK() const {return m_softBody->m_cfg.kSK_SPLT_CL;}
float PhysSoftBody::GetSoftVsRigidHardness() const {return m_softBody->m_cfg.kSRHR_CL;}
float PhysSoftBody::GetSoftVsRigidImpulseSplitR() const {return m_softBody->m_cfg.kSR_SPLT_CL;}
float PhysSoftBody::GetSoftVsSoftHardness() const {return m_softBody->m_cfg.kSSHR_CL;}
float PhysSoftBody::GetSoftVsRigidImpulseSplitS() const {return m_softBody->m_cfg.kSS_SPLT_CL;}
float PhysSoftBody::GetVolumeConversationCoefficient() const {return m_softBody->m_cfg.kVC;}
float PhysSoftBody::GetVelocitiesCorrectionFactor() const {return m_softBody->m_cfg.kVCF;}

void PhysSoftBody::SetMaterialAngularStiffnessCoefficient(uint32_t matId,float val)
{
	if(matId >= m_softBody->m_materials.size())
		return;
	m_softBody->m_materials.at(matId)->m_kAST = val;
}
void PhysSoftBody::SetMaterialLinearStiffnessCoefficient(uint32_t matId,float val)
{
	if(matId >= m_softBody->m_materials.size())
		return;
	m_softBody->m_materials.at(matId)->m_kLST = val;
}
void PhysSoftBody::SetMaterialVolumeStiffnessCoefficient(uint32_t matId,float val)
{
	if(matId >= m_softBody->m_materials.size())
		return;
	m_softBody->m_materials.at(matId)->m_kVST = val;
}
float PhysSoftBody::GetMaterialAngularStiffnessCoefficient(uint32_t matId) const
{
	if(matId >= m_softBody->m_materials.size())
		return 0.f;
	return m_softBody->m_materials.at(matId)->m_kAST;
}
float PhysSoftBody::GetMaterialLinearStiffnessCoefficient(uint32_t matId) const
{
	if(matId >= m_softBody->m_materials.size())
		return 0.f;
	return m_softBody->m_materials.at(matId)->m_kLST;
}
float PhysSoftBody::GetMaterialVolumeStiffnessCoefficient(uint32_t matId) const
{
	if(matId >= m_softBody->m_materials.size())
		return 0.f;
	return m_softBody->m_materials.at(matId)->m_kVST;
}

void PhysSoftBody::AddWorldObject()
{
	if(m_bSpawned == false)
		return;
	RemoveWorldObject();
#if PHYS_USE_SOFT_RIGID_DYNAMICS_WORLD == 1
	auto *world = m_physEnv->GetWorld();
	world->addSoftBody(m_softBody,umath::to_integral(m_collisionFilterGroup),umath::to_integral(m_collisionFilterMask));
#endif
}

void PhysSoftBody::RemoveWorldObject()
{
#if PHYS_USE_SOFT_RIGID_DYNAMICS_WORLD == 1
	auto *world = m_physEnv->GetWorld();
	world->removeSoftBody(m_softBody);
#endif
}

btSoftBody *PhysSoftBody::GetSoftBody() {return m_softBody;}

Vector3 PhysSoftBody::GetPos() const
{
	Vector3 min,max;
	GetAABB(min,max);
	return (min +max) *0.5f;
}

void PhysSoftBody::SetPos(const Vector3 &pos)
{
	auto posCur = GetPos();
	auto t = m_softBody->getWorldTransform();
	t.setOrigin(uvec::create_bt(posCur) *PhysEnv::WORLD_SCALE);
	auto inv = t.inverse();
	t.setOrigin(uvec::create_bt(pos) *PhysEnv::WORLD_SCALE);
	m_softBody->transform(inv *t);
}
void PhysSoftBody::SetRotation(const Quat &rot)
{
	auto transformRot = uquat::get_inverse(m_rotation) *rot;
	btTransform t = {};
	t.setIdentity();
	t.setRotation(uquat::create_bt(transformRot));
	m_softBody->transform(t);

	m_rotation = rot;
}

Quat PhysSoftBody::GetRotation() const {return m_rotation;}

void PhysSoftBody::SetWorldTransform(const PhysTransform &t)
{
	auto &bt = t.GetTransform();
	m_softBody->transform(m_softBody->getWorldTransform().inverse() *bt);
}

bool PhysSoftBody::IsSoftBody() const {return true;}

//////////////////////////

PhysGhostObject::PhysGhostObject(PhysEnv *env,btPairCachingGhostObject *o,std::shared_ptr<PhysShape> &shape)
	: PhysCollisionObject(env,o,shape),m_ghostObject(o)
{}

void PhysGhostObject::AddWorldObject()
{
	if(m_bSpawned == false)
		return;
	RemoveWorldObject();
	auto *world = m_physEnv->GetWorld();
	world->addCollisionObject(m_collisionObject,umath::to_integral(m_collisionFilterGroup),umath::to_integral(m_collisionFilterMask));//,btBroadphaseProxy::CharacterFilter,btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
}

void PhysGhostObject::RemoveWorldObject()
{
	auto *world = m_physEnv->GetWorld();
	world->removeCollisionObject(m_collisionObject);
}

bool PhysGhostObject::IsGhost() const {return true;}

btPairCachingGhostObject *PhysGhostObject::GetGhostObject() {return m_ghostObject;}

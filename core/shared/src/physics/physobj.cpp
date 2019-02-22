#include "stdafx_shared.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/physcontroller.h"
#include "pragma/physics/physshape.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/entities/components/base_physics_component.hpp"

DEFINE_BASE_HANDLE(DLLNETWORK,PhysObj,PhysObj);

PhysObj::PhysObj(pragma::BaseEntityComponent *owner)
	: m_handle(new PtrPhysObj(this)),
	m_collisionFilterGroup(CollisionMask::None),m_collisionFilterMask(CollisionMask::None)
{
	m_owner = owner->GetHandle<pragma::BaseEntityComponent>();
	m_networkState = owner->GetEntity().GetNetworkState();
}

#ifdef PHYS_ENGINE_BULLET
PhysObj::PhysObj(pragma::BaseEntityComponent *owner,PhysCollisionObject *object)
	: PhysObj(owner)
{
	AddCollisionObject(object);
}

PhysObj::PhysObj(pragma::BaseEntityComponent *owner,std::vector<PhysCollisionObject*> *objects)
	: PhysObj(owner)
{
	for(unsigned int i=0;i<objects->size();i++)
		AddCollisionObject((*objects)[i]);
}
void PhysObj::GetAABB(Vector3 &min,Vector3 &max) const
{
	min = {std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};
	max = {std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest()};
	auto b = false;
	Vector3 oMin,oMax;
	for(auto &hObj : m_collisionObjects)
	{
		if(hObj.IsValid())
		{
			b = true;
			auto &o = *hObj.get();
			o.GetAABB(oMin,oMax);
			assert(oMin.x <= oMax.x && oMin.y <= oMax.y && oMin.z <= oMax.z);
			uvec::min(&min,oMin);
			uvec::max(&max,oMax);
		}
	}
	if(b == false)
	{
		min = {};
		max = {};
		return;
	}
}
Vector3 PhysObj::GetLinearVelocity() {return m_velocity;}
void PhysObj::UpdateVelocity() {}
NetworkState *PhysObj::GetNetworkState() {return m_networkState;}
float PhysObj::GetMass() const {return 0.f;}
void PhysObj::SetMass(float) {}
bool PhysObj::IsDisabled() const {return m_bDisabled;}
bool PhysObj::IsStatic() const {return false;}
void PhysObj::SetStatic(bool) {}
bool PhysObj::IsRigid() const {return false;}
bool PhysObj::IsSoftBody() const {return false;}

void PhysObj::UpdateCCD()
{
	for(auto &hObj : m_collisionObjects)
	{
		if(hObj.IsValid() == false)
			continue;
		hObj->UpdateCCD();
	}
}

void PhysObj::SetCCDEnabled(bool b)
{
	for(auto &hObj : m_collisionObjects)
	{
		if(hObj.IsValid() == false)
			continue;
		hObj->SetCCDEnabled(b);
		hObj->UpdateCCD();
	}
}

void PhysObj::AddCollisionObject(PhysCollisionObject *obj)
{
	m_collisionObjects.push_back(obj->GetHandle());
	obj->userData = this;
	if(m_bSpawned == true)
		obj->Spawn();
}

void PhysObj::Spawn()
{
	if(m_bSpawned == true)
		return;
	for(auto &hObj : m_collisionObjects)
	{
		if(hObj.IsValid())
			hObj->Spawn();
	}
}

std::vector<PhysCollisionObjectHandle> &PhysObj::GetCollisionObjects() {return m_collisionObjects;}
PhysCollisionObject *PhysObj::GetCollisionObject()
{
	if(m_collisionObjects.empty())
		return nullptr;
	auto &hObj = m_collisionObjects.front();
	if(!hObj.IsValid())
		return nullptr;
	return hObj.get();
}
#elif PHYS_ENGINE_PHYSX
PhysObj::PhysObj(BaseEntity *owner,physx::PxRigidActor *actor)
	: PhysObj()
{
	m_owner = owner->CreateHandle();
	AddActor(actor);
}

PhysObj::PhysObj(BaseEntity *owner,std::vector<physx::PxRigidActor*> *actors)
	: PhysObj()
{
	m_owner = owner->CreateHandle();
	for(unsigned int i=0;i<actors->size();i++)
		AddActor((*actors)[i]);
}

void PhysObj::AddActor(physx::PxRigidActor *actor)
{
	m_actors.push_back(actor);
	actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY,1);
	actor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES,1);
	actor->userData = this;
}

physx::PxRigidActor *PhysObj::GetActor()
{
	if(m_actors.empty())
		return NULL;
	return m_actors[0];
}
#endif

PhysObj::~PhysObj()
{
	m_handle.Invalidate();
#ifdef PHYS_ENGINE_PHYSX
	for(unsigned int i=0;i<m_actors.size();i++)
		m_actors[i]->release();
#endif
#ifdef PHYS_ENGINE_BULLET
	//NetworkState *state = m_networkState;
	for(unsigned int i=0;i<m_collisionObjects.size();i++)
	{
		auto &o = m_collisionObjects[i];
		if(o.IsValid())
			delete o.get();
	}
#endif
}
void PhysObj::OnSleep()
{
	if(m_owner.expired())
		return;
	m_bAsleep = true;
	auto pPhysComponent = m_owner->GetEntity().GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->OnPhysicsSleep(this);
}
void PhysObj::OnWake()
{
	if(m_owner.expired())
		return;
	m_bAsleep = false;
	auto pPhysComponent = m_owner->GetEntity().GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->OnPhysicsWake(this);
}
void PhysObj::Enable() {m_bDisabled = false;}
void PhysObj::Disable() {m_bDisabled = true;}
pragma::BaseEntityComponent *PhysObj::GetOwner() {return m_owner.get();}
bool PhysObj::IsController() {return false;}

void PhysObj::SetLinearFactor(const Vector3&) {}
void PhysObj::SetAngularFactor(const Vector3&) {}
Vector3 PhysObj::GetLinearFactor() const {return Vector3(0.f,0.f,0.f);}
Vector3 PhysObj::GetAngularFactor() const {return Vector3(0.f,0.f,0.f);}

void PhysObj::SetLinearSleepingThreshold(float threshold) {SetSleepingThresholds(threshold,GetAngularSleepingThreshold());}
void PhysObj::SetAngularSleepingThreshold(float threshold) {SetSleepingThresholds(GetLinearSleepingThreshold(),threshold);}
std::pair<float,float> PhysObj::GetSleepingThreshold() const {return {GetLinearSleepingThreshold(),GetAngularSleepingThreshold()};}
void PhysObj::SetSleepingThresholds(float linear,float angular) {}
float PhysObj::GetLinearSleepingThreshold() const {return 0.f;}
float PhysObj::GetAngularSleepingThreshold() const {return 0.f;}

void PhysObj::Simulate(double,bool) {}

PhysObjHandle PhysObj::GetHandle() {return m_handle;}
PhysObjHandle *PhysObj::CreateHandle() {return m_handle.Copy();}

#ifdef PHYS_ENGINE_BULLET
void PhysObj::SetPosition(const Vector3 &pos)
{
	if(m_collisionObjects.empty())
		return;
	auto &root = m_collisionObjects[0];
	if(!root.IsValid())
		return;

	auto r = pos;
	/*if(root->HasOrigin())
	{
		auto rot = root->GetRotation();
		auto &origin = root->GetOrigin();
		r -= origin *uquat::get_inverse(rot);
	}*/

	Vector3 posRoot = root->GetPos();
	for(unsigned int i=0;i<m_collisionObjects.size();i++)
	{
		auto &o = m_collisionObjects[i];
		if(o.IsValid())
		{
			Vector3 posOther = o->GetPos();
			o->SetPos(r +(posOther -posRoot));
		}
	}
}
void PhysObj::SetOrientation(const Quat &q)
{
	if(m_collisionObjects.empty())
		return;
	auto &root = m_collisionObjects.front();
	if(!root.IsValid())
		return;
	auto oldRot = root->GetRotation();
	auto origin = root->GetPos();

	auto rotOffset = uquat::get_inverse(oldRot) *q;
	for(auto it=m_collisionObjects.begin();it!=m_collisionObjects.end();++it)
	{
		auto &o = *it;
		if(o.IsValid() && o.get() != root.get())
		{
			auto offset = o->GetPos() -origin;
			uvec::rotate(&offset,rotOffset);
			o->SetRotation(rotOffset *o->GetRotation());
			o->SetPos(origin +offset);
		}
	}
	root->SetRotation(q); // Faster and less prone to precision errors
}
Quat PhysObj::GetOrientation()
{
	PhysCollisionObject *o = GetCollisionObject();
	if(o == NULL)
		return uquat::identity();
	return o->GetRotation();
}
Vector3 PhysObj::GetPosition()
{
	PhysCollisionObject *o = GetCollisionObject();
	if(o == NULL)
		return Vector3(0,0,0);
	auto r = o->GetPos();
	//return r;
	if(!o->HasOrigin()) // If origin is enabled, position can be vastly off from the entity position (e.g. player ragdoll)
		return r;
	auto rot = o->GetRotation();
	auto &origin = o->GetOrigin();
	r += origin *uquat::get_inverse(rot);
	return r;
}
Vector3 PhysObj::GetOrigin() const
{
	PhysCollisionObject *o = const_cast<PhysObj*>(this)->GetCollisionObject();
	if(o == NULL)
		return Vector3(0,0,0);
	auto r = o->GetPos();
	if(!o->HasOrigin()) // If origin is enabled, position can be vastly off from the entity position (e.g. player ragdoll)
		return r;
	auto rot = o->GetRotation();
	auto &origin = o->GetOrigin();
	r += origin *uquat::get_inverse(rot);
	return r;
}
void PhysObj::SetTrigger(bool b)
{
	m_bTrigger = b;
	for(unsigned int i=0;i<m_collisionObjects.size();i++)
	{
		auto &o = m_collisionObjects[i];
		if(o.IsValid())
			o->SetTrigger(b);
	}
}
void PhysObj::SetCollisionFilter(CollisionMask filterGroup,CollisionMask filterMask)
{
	m_collisionFilterGroup = filterGroup;
	m_collisionFilterMask = filterMask;
	for(auto &hObj : m_collisionObjects)
	{
		if(hObj.IsValid())
		{
			hObj->SetCollisionFilterGroup(filterGroup);
			hObj->SetCollisionFilterMask(filterMask);
		}
	}
}
void PhysObj::SetCollisionFilterMask(CollisionMask filterMask) {SetCollisionFilter(m_collisionFilterGroup,filterMask);}
#elif PHYS_ENGINE_PHYSX
void PhysObj::SetPosition(const Vector3 &pos)
{
	if(m_actors.empty())
		return;
	physx::PxRigidActor *root = m_actors[0];
	physx::PxTransform tRoot = root->getGlobalPose();
	physx::PxVec3 &posRoot = tRoot.p;
	for(unsigned int i=0;i<m_actors.size();i++)
	{
		physx::PxRigidActor *actor = m_actors[i];
		physx::PxTransform t = actor->getGlobalPose();
		t.p.x = pos.x +(t.p.x -posRoot.x);
		t.p.y = pos.y +(t.p.y -posRoot.y);
		t.p.z = pos.z +(t.p.z -posRoot.z);
		actor->setGlobalPose(t);
	}
}
void PhysObj::SetOrientation(const Quat &q)
{
	if(m_actors.empty())
		return;
	physx::PxRigidActor *root = m_actors[0];
	physx::PxTransform tRoot = root->getGlobalPose();
	physx::PxQuat &rotRoot = tRoot.q;
	physx::PxQuat qNew(q.x,q.y,q.z,q.w);
	for(unsigned int i=0;i<m_actors.size();i++)
	{
		physx::PxRigidActor *actor = m_actors[i];
		physx::PxTransform t = actor->getGlobalPose();
		t.q = t.q *rotRoot.getConjugate() *qNew;
		actor->setGlobalPose(t);
	}
}
Quat PhysObj::GetOrientation()
{
	physx::PxRigidActor *actor = GetActor();
	if(actor == NULL)
		return uquat::identity();
	physx::PxTransform t = actor->getGlobalPose();
	return Quat(t.q.w,t.q.x,t.q.y,t.q.z);
}
Vector3 PhysObj::GetPosition()
{
	physx::PxRigidActor *actor = GetActor();
	if(actor == NULL)
		return Vector3(0,0,0);
	physx::PxTransform t = actor->getGlobalPose();
	return Vector3(t.p.x,t.p.y,t.p.z);
}
void PhysObj::SetTrigger(bool b)
{
	m_bTrigger = b;
	for(unsigned int i=0;i<m_actors.size();i++)
	{
		physx::PxRigidActor *actor = m_actors[i];
		unsigned int numShapes = actor->getNbShapes();
		physx::PxShape **shapes = new physx::PxShape*[numShapes];
		actor->getShapes(&shapes[0],numShapes);
		for(unsigned int j=0;j<numShapes;j++)
		{
			physx::PxShape *shape = shapes[j];
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE,!b);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE,b);
		}
		delete[] shapes;
	}
}
void PhysObj::SetCollisionFilter(CollisionMask filterGroup,CollisionMask filterMask)
{
	m_collisionFilterGroup = filterGroup;
	m_collisionFilterMask = filterMask;
	for(unsigned int i=0;i<m_actors.size();i++)
	{
		physx::PxRigidActor *actor = m_actors[i];
		unsigned int numShapes = actor->getNbShapes();
		physx::PxShape **shapes = new physx::PxShape*[numShapes];
		actor->getShapes(&shapes[0],numShapes);
		for(unsigned int j=0;j<numShapes;j++)
		{
			physx::PxShape *shape = shapes[j];
			physx::PxFilterData filter = shape->getSimulationFilterData();
			filter.word0 = filterGroup;
			filter.word1 = filterMask;
			shape->setSimulationFilterData(filter);
		}
		delete[] shapes;
	}
}
std::vector<physx::PxRigidActor*> *PhysObj::GetActors() {return &m_actors;}
#endif
void PhysObj::AddCollisionFilter(CollisionMask filter)
{
	CollisionMask filterGroup;
	CollisionMask filterMask;
	GetCollisionFilter(&filterGroup,&filterMask);
	SetCollisionFilter(filterGroup | filter,filterMask | filter);
}
void PhysObj::RemoveCollisionFilter(CollisionMask filter)
{
	CollisionMask filterGroup;
	CollisionMask filterMask;
	GetCollisionFilter(&filterGroup,&filterMask);
	SetCollisionFilter(filterGroup & ~filter,filterMask & ~filter);
}
void PhysObj::SetCollisionFilter(CollisionMask filterGroup) {SetCollisionFilter(filterGroup,filterGroup);}
CollisionMask PhysObj::GetCollisionFilter() {return m_collisionFilterGroup;}
CollisionMask PhysObj::GetCollisionFilterMask() {return m_collisionFilterMask;}
void PhysObj::GetCollisionFilter(CollisionMask *filterGroup,CollisionMask *filterMask)
{
	*filterGroup = m_collisionFilterGroup;
	*filterMask = m_collisionFilterMask;
}

bool PhysObj::IsTrigger() {return m_bTrigger;}

void PhysObj::SetLinearVelocity(const Vector3&) {}
void PhysObj::AddLinearVelocity(const Vector3 &vel) {SetLinearVelocity(GetLinearVelocity() +vel);}
Vector3 PhysObj::GetAngularVelocity() {return Vector3(0,0,0);}
void PhysObj::SetAngularVelocity(const Vector3&) {}
void PhysObj::AddAngularVelocity(const Vector3 &vel) {SetAngularVelocity(GetAngularVelocity() +vel);}
void PhysObj::PutToSleep() {}
void PhysObj::WakeUp() {}
bool PhysObj::IsSleeping() {return m_bAsleep;}
void PhysObj::SetDamping(float,float) {}
void PhysObj::SetLinearDamping(float) {}
void PhysObj::SetAngularDamping(float) {}
float PhysObj::GetLinearDamping() const {return 0.f;}
float PhysObj::GetAngularDamping() const {return 0.f;}

void PhysObj::ApplyForce(const Vector3&) {}
void PhysObj::ApplyForce(const Vector3&,const Vector3&) {}
void PhysObj::ApplyImpulse(const Vector3&) {}
void PhysObj::ApplyImpulse(const Vector3&,const Vector3&) {}
void PhysObj::ApplyTorque(const Vector3&) {}
void PhysObj::ApplyTorqueImpulse(const Vector3&) {}
void PhysObj::ClearForces() {}
Vector3 PhysObj::GetTotalForce() {return Vector3(0.f,0.f,0.f);}
Vector3 PhysObj::GetTotalTorque() {return Vector3(0.f,0.f,0.f);}

////////////////////////////////////

#if PHYS_ENGINE_PHYSX
DynamicActorInfo::DynamicActorInfo(physx::PxRigidDynamic *actor,int bone)
	: m_actor(actor),m_bone(bone)
{}
physx::PxRigidDynamic *DynamicActorInfo::GetActor() {return m_actor;}
int DynamicActorInfo::GetBoneID() {return m_bone;}

DynamicPhysObj::DynamicPhysObj(BaseEntity *owner,physx::PxRigidDynamic *actor,int bone)
	: PhysObj(owner,actor),PhysObjGravitation(),PhysObjKinematic()
{
	m_actorInfos.push_back(DynamicActorInfo(actor,bone));
	m_bAsleep = actor->isSleeping();
}

DynamicPhysObj::DynamicPhysObj(BaseEntity *owner,std::vector<physx::PxRigidActor*> *actors)
	: PhysObj(owner,actors)
{
	for(unsigned int i=0;i<actors->size();i++)
		m_actorInfos.push_back(DynamicActorInfo(static_cast<physx::PxRigidDynamic*>((*actors)[i])));
	if(!m_actorInfos.empty())
		m_bAsleep = m_actorInfos[0].GetActor()->isSleeping();
}
void DynamicPhysObj::AddActor(physx::PxRigidActor *actor)
{
	PhysObj::AddActor(actor);
}
unsigned int DynamicPhysObj::AddActor(physx::PxRigidDynamic *actor,int bone)
{
	AddActor(static_cast<physx::PxRigidActor*>(actor));
	m_actorInfos.push_back(DynamicActorInfo(actor,bone));
	return m_actorInfos.size() -1;
}
unsigned int DynamicPhysObj::AddActor(physx::PxRigidDynamic *actor) {return AddActor(actor,0);}
int DynamicPhysObj::GetBoneID(physx::PxRigidDynamic *actor)
{
	for(unsigned int i=0;i<m_actorInfos.size();i++)
	{
		if(m_actorInfos[i].GetActor() == actor)
			return m_actorInfos[i].GetBoneID();
	}
	return -1;
}
void DynamicPhysObj::PutToSleep()
{
	for(unsigned int i=0;i<m_actorInfos.size();i++)
		m_actorInfos[i].GetActor()->putToSleep();
}
void DynamicPhysObj::WakeUp()
{
	for(unsigned int i=0;i<m_actorInfos.size();i++)
		m_actorInfos[i].GetActor()->wakeUp();
}
void DynamicPhysObj::Simulate(double tDelta,bool bIgnoreGravity)
{
	if(m_bDisabled == true || IsKinematic())
		return;
	if(!m_owner->IsValid())
		return;
	if(bIgnoreGravity)
		return;
	Vector3 f = GetGravityForce();
	physx::PxVec3 force(f.x,f.y,f.z);
	for(unsigned int i=0;i<m_actorInfos.size();i++)
		m_actorInfos[i].GetActor()->addForce(force,physx::PxForceMode::eACCELERATION,false);
}
Vector3 DynamicPhysObj::GetLinearVelocity()
{
	physx::PxRigidDynamic *actor = static_cast<physx::PxRigidDynamic*>(GetActor());
	if(actor == NULL)
		return Vector3(0,0,0);
	physx::PxVec3 vel = actor->getLinearVelocity();
	return Vector3(vel.x,vel.y,vel.z);
}

void DynamicPhysObj::SetLinearVelocity(const Vector3 &vel)
{
	if(IsKinematic())
		return;
	for(unsigned int i=0;i<m_actorInfos.size();i++)
		m_actorInfos[i].GetActor()->setLinearVelocity(physx::PxVec3(vel.x,vel.y,vel.z));
}
void DynamicPhysObj::SetKinematic(bool b)
{
	PhysObjKinematic::SetKinematic(b,m_actors);
}
Vector3 DynamicPhysObj::GetAngularVelocity()
{
	physx::PxRigidDynamic *actor = static_cast<physx::PxRigidDynamic*>(GetActor());
	if(actor == NULL)
		return Vector3(0,0,0);
	physx::PxVec3 vel = actor->getAngularVelocity();
	return Vector3(vel.x,vel.y,vel.z);
}
void DynamicPhysObj::SetAngularVelocity(const Vector3 &vel)
{
	if(IsKinematic())
		return;
	for(unsigned int i=0;i<m_actorInfos.size();i++)
		m_actorInfos[i].GetActor()->setAngularVelocity(physx::PxVec3(vel.x,vel.y,vel.z));
}

DynamicPhysObj::~DynamicPhysObj()
{}

DynamicActorInfo *DynamicPhysObj::GetActorInfo(unsigned int idx)
{
	if(idx >= m_actorInfos.size())
		return NULL;
	return &m_actorInfos[idx];
}
std::vector<DynamicActorInfo> &DynamicPhysObj::GetActorInfo() {return m_actorInfos;}

BaseEntity *DynamicPhysObj::GetOwner() {return PhysObj::GetOwner();}

void DynamicPhysObj::AddLinearVelocity(const Vector3 &vel) {SetLinearVelocity(GetLinearVelocity() +vel);}
void DynamicPhysObj::AddAngularVelocity(const Vector3 &vel) {SetAngularVelocity(GetAngularVelocity() +vel);}

////////////////////////////////////

StaticPhysObj::StaticPhysObj(BaseEntity *owner,physx::PxRigidStatic *actor)
	: PhysObj(owner,actor)
{
	m_staticActors.push_back(actor);
}

StaticPhysObj::StaticPhysObj(BaseEntity *owner,std::vector<physx::PxRigidActor*> *actors)
	: PhysObj(owner,actors)
{
	for(unsigned int i=0;i<actors->size();i++)
		m_staticActors.push_back(static_cast<physx::PxRigidStatic*>((*actors)[i]));
}
bool StaticPhysObj::IsStatic() {return true;}
void StaticPhysObj::Enable() {}
void StaticPhysObj::Disable() {}
#endif

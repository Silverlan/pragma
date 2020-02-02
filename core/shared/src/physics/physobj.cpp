#include "stdafx_shared.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/physics/collision_object.hpp"
#include "pragma/physics/controller.hpp"
#include "pragma/physics/shape.hpp"
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

PhysObj::PhysObj(pragma::BaseEntityComponent *owner,pragma::physics::ICollisionObject &object)
	: PhysObj(owner)
{
	AddCollisionObject(object);
}

PhysObj::PhysObj(pragma::BaseEntityComponent *owner,const std::vector<pragma::physics::ICollisionObject*> &objects)
	: PhysObj(owner)
{
	for(unsigned int i=0;i<objects.size();i++)
		AddCollisionObject(*objects[i]);
}
bool PhysObj::Initialize() {return true;}
void PhysObj::OnCollisionObjectWake(pragma::physics::ICollisionObject &o)
{
	if(m_colObjAwakeCount++ == 0)
		OnWake();

	// Sanity check
	if(m_colObjAwakeCount > m_collisionObjects.size())
		Con::cwar<<"WARNING: Collision object wake counter exceeds number of collision objects!"<<Con::endl;
}
void PhysObj::OnCollisionObjectSleep(pragma::physics::ICollisionObject &o)
{
	if(m_colObjAwakeCount == 0)
	{
		Con::cwar<<"WARNING: Collision object of physics object fell asleep, but previous information indicated all collision objects were already asleep!"<<Con::endl;
		return;
	}
	if(--m_colObjAwakeCount == 0)
		OnSleep();
}
void PhysObj::OnCollisionObjectRemoved(pragma::physics::ICollisionObject &o)
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::Spawned) && o.IsAwake())
		OnCollisionObjectSleep(o);
	auto it = std::find_if(m_collisionObjects.begin(),m_collisionObjects.end(),[&o](const util::TSharedHandle<pragma::physics::ICollisionObject> &colObjOther) {
		return &o == colObjOther.Get();
	});
	if(it != m_collisionObjects.end())
		m_collisionObjects.erase(it);
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
			auto &o = *hObj.Get();
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
Vector3 PhysObj::GetLinearVelocity() const {return m_velocity;}
void PhysObj::UpdateVelocity() {}
NetworkState *PhysObj::GetNetworkState() {return m_networkState;}
float PhysObj::GetMass() const {return 0.f;}
void PhysObj::SetMass(float) {}
bool PhysObj::IsDisabled() const {return umath::is_flag_set(m_stateFlags,StateFlags::Disabled);}
bool PhysObj::IsStatic() const {return false;}
void PhysObj::SetStatic(bool) {}
bool PhysObj::IsRigid() const {return false;}
bool PhysObj::IsSoftBody() const {return false;}

void PhysObj::SetCCDEnabled(bool b)
{
	for(auto &hObj : m_collisionObjects)
	{
		if(hObj.IsValid() == false)
			continue;
		hObj->SetCCDEnabled(b);
	}
}

void PhysObj::AddCollisionObject(pragma::physics::ICollisionObject &obj)
{
	m_collisionObjects.push_back(util::shared_handle_cast<pragma::physics::IBase,pragma::physics::ICollisionObject>(obj.ClaimOwnership()));
	obj.SetPhysObj(*this);
	if(umath::is_flag_set(m_stateFlags,StateFlags::Spawned))
		obj.Spawn();
}

void PhysObj::Spawn()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::Spawned))
		return;
	for(auto &hObj : m_collisionObjects)
	{
		if(hObj.IsValid())
			hObj->Spawn();
	}
}
const pragma::physics::ICollisionObject *PhysObj::GetCollisionObject() const {return const_cast<PhysObj*>(this)->GetCollisionObject();}
std::vector<util::TSharedHandle<pragma::physics::ICollisionObject>> &PhysObj::GetCollisionObjects() {return m_collisionObjects;}
pragma::physics::ICollisionObject *PhysObj::GetCollisionObject()
{
	if(m_collisionObjects.empty())
		return nullptr;
	auto &hObj = m_collisionObjects.front();
	if(!hObj.IsValid())
		return nullptr;
	return hObj.Get();
}

PhysObj::~PhysObj()
{
	m_handle.Invalidate();
	//NetworkState *state = m_networkState;
	for(unsigned int i=0;i<m_collisionObjects.size();i++)
	{
		auto &o = m_collisionObjects[i];
		if(o.IsValid())
			o.Remove();
	}
}
void PhysObj::OnSleep()
{
	if(m_owner.expired())
		return;
	auto pPhysComponent = m_owner->GetEntity().GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->OnPhysicsSleep(this);
}
void PhysObj::OnWake()
{
	if(m_owner.expired())
		return;
	auto pPhysComponent = m_owner->GetEntity().GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->OnPhysicsWake(this);
}
void PhysObj::Enable() {umath::set_flag(m_stateFlags,StateFlags::Disabled,false);}
void PhysObj::Disable() {umath::set_flag(m_stateFlags,StateFlags::Disabled,true);}
pragma::BaseEntityComponent *PhysObj::GetOwner() {return m_owner.get();}
bool PhysObj::IsController() const {return false;}

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

PhysObjHandle PhysObj::GetHandle() const {return m_handle;}

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
		if(o.IsValid() && o.Get() != root.Get())
		{
			auto offset = o->GetPos() -origin;
			uvec::rotate(&offset,rotOffset);
			o->SetRotation(rotOffset *o->GetRotation());
			o->SetPos(origin +offset);
		}
	}
	root->SetRotation(q); // Faster and less prone to precision errors
}
Quat PhysObj::GetOrientation() const
{
	auto *o = GetCollisionObject();
	if(o == NULL)
		return uquat::identity();
	return o->GetRotation();
}
Vector3 PhysObj::GetPosition() const
{
	auto *o = GetCollisionObject();
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
	auto *o = const_cast<PhysObj*>(this)->GetCollisionObject();
	if(o == nullptr)
		return Vector3(0,0,0);
	auto r = o->GetPos();
	if(!o->HasOrigin()) // If origin is enabled, position can be vastly off from the entity position (e.g. player ragdoll)
		return r;
	auto rot = o->GetRotation();
	auto &origin = o->GetOrigin();
	r += origin *uquat::get_inverse(rot);
	return r;
}
uint32_t PhysObj::GetNumberOfCollisionObjectsAwake() const {return m_colObjAwakeCount;}
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
CollisionMask PhysObj::GetCollisionFilter() const {return m_collisionFilterGroup;}
CollisionMask PhysObj::GetCollisionFilterMask() const {return m_collisionFilterMask;}
void PhysObj::GetCollisionFilter(CollisionMask *filterGroup,CollisionMask *filterMask) const
{
	*filterGroup = m_collisionFilterGroup;
	*filterMask = m_collisionFilterMask;
}

bool PhysObj::IsTrigger() const
{
	auto *colObj = GetCollisionObject();
	return colObj && colObj->IsTrigger();
}
void PhysObj::SetTrigger(bool bTrigger)
{
	for(auto &hColObj : m_collisionObjects)
	{
		if(hColObj.IsExpired())
			continue;
		hColObj->SetTrigger(bTrigger);
	}
}

void PhysObj::SetLinearVelocity(const Vector3&) {}
void PhysObj::AddLinearVelocity(const Vector3 &vel) {SetLinearVelocity(GetLinearVelocity() +vel);}
Vector3 PhysObj::GetAngularVelocity() const {return Vector3(0,0,0);}
void PhysObj::SetAngularVelocity(const Vector3&) {}
void PhysObj::AddAngularVelocity(const Vector3 &vel) {SetAngularVelocity(GetAngularVelocity() +vel);}
void PhysObj::PutToSleep() {}
void PhysObj::WakeUp() {}
bool PhysObj::IsSleeping() const {return m_colObjAwakeCount == m_collisionObjects.size();}
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
Vector3 PhysObj::GetTotalForce() const {return Vector3(0.f,0.f,0.f);}
Vector3 PhysObj::GetTotalTorque() const {return Vector3(0.f,0.f,0.f);}

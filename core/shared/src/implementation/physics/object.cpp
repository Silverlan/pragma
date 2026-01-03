// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :physics.object;

pragma::physics::PhysObj::PhysObj(BaseEntityComponent *owner) : BaseLuaHandle {}, m_collisionFilterGroup(CollisionMask::None), m_collisionFilterMask(CollisionMask::None)
{
	m_owner = owner->GetHandle<BaseEntityComponent>();
	m_networkState = owner->GetEntity().GetNetworkState();
}

pragma::physics::PhysObj::PhysObj(BaseEntityComponent *owner, ICollisionObject &object) : PhysObj(owner) { AddCollisionObject(object); }

pragma::physics::PhysObj::PhysObj(BaseEntityComponent *owner, const std::vector<ICollisionObject *> &objects) : PhysObj(owner)
{
	for(unsigned int i = 0; i < objects.size(); i++)
		AddCollisionObject(*objects[i]);
}
bool pragma::physics::PhysObj::Initialize()
{
	InitializeLuaObject(GetNetworkState()->GetLuaState());
	return true;
}
void pragma::physics::PhysObj::InitializeLuaObject(lua::State *lua) { SetLuaObject(LuaCore::raw_object_to_luabind_object(lua, GetHandle())); }
void pragma::physics::PhysObj::OnCollisionObjectWake(ICollisionObject &o)
{
	if(m_colObjAwakeCount++ == 0)
		OnWake();

	// Sanity check
	if(m_colObjAwakeCount > m_collisionObjects.size())
		Con::CWAR << "Collision object wake counter exceeds number of collision objects!" << Con::endl;
}
void pragma::physics::PhysObj::OnCollisionObjectSleep(ICollisionObject &o)
{
	if(m_colObjAwakeCount == 0) {
		Con::CWAR << "Collision object of physics object fell asleep, but previous information indicated all collision objects were already asleep!" << Con::endl;
		return;
	}
	if(--m_colObjAwakeCount == 0)
		OnSleep();
}
void pragma::physics::PhysObj::OnCollisionObjectRemoved(ICollisionObject &o)
{
	if(math::is_flag_set(m_stateFlags, StateFlags::Spawned) && o.IsAwake())
		OnCollisionObjectSleep(o);
	auto it = std::find_if(m_collisionObjects.begin(), m_collisionObjects.end(), [&o](const util::TSharedHandle<ICollisionObject> &colObjOther) { return &o == colObjOther.Get(); });
	if(it != m_collisionObjects.end())
		m_collisionObjects.erase(it);
}
void pragma::physics::PhysObj::GetAABB(Vector3 &min, Vector3 &max) const
{
	min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	max = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
	auto b = false;
	Vector3 oMin, oMax;
	for(auto &hObj : m_collisionObjects) {
		if(hObj.IsValid()) {
			b = true;
			auto &o = *hObj.Get();
			o.GetAABB(oMin, oMax);
			assert(oMin.x <= oMax.x && oMin.y <= oMax.y && oMin.z <= oMax.z);
			uvec::min(&min, oMin);
			uvec::max(&max, oMax);
		}
	}
	if(b == false) {
		min = {};
		max = {};
		return;
	}
}
Vector3 pragma::physics::PhysObj::GetLinearVelocity() const { return m_velocity; }
void pragma::physics::PhysObj::UpdateVelocity() {}
pragma::NetworkState *pragma::physics::PhysObj::GetNetworkState() { return m_networkState; }
float pragma::physics::PhysObj::GetMass() const { return 0.f; }
void pragma::physics::PhysObj::SetMass(float) {}
bool pragma::physics::PhysObj::IsDisabled() const { return math::is_flag_set(m_stateFlags, StateFlags::Disabled); }
bool pragma::physics::PhysObj::IsStatic() const { return false; }
void pragma::physics::PhysObj::SetStatic(bool) {}
bool pragma::physics::PhysObj::IsRigid() const { return false; }
bool pragma::physics::PhysObj::IsSoftBody() const { return false; }

void pragma::physics::PhysObj::SetCCDEnabled(bool b)
{
	for(auto &hObj : m_collisionObjects) {
		if(hObj.IsValid() == false)
			continue;
		hObj->SetCCDEnabled(b);
	}
}

void pragma::physics::PhysObj::AddCollisionObject(ICollisionObject &obj)
{
	m_collisionObjects.push_back(pragma::util::shared_handle_cast<IBase, ICollisionObject>(obj.ClaimOwnership()));
	obj.SetPhysObj(*this);
	if(math::is_flag_set(m_stateFlags, StateFlags::Spawned))
		obj.Spawn();
}

void pragma::physics::PhysObj::Spawn()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::Spawned))
		return;
	for(auto &hObj : m_collisionObjects) {
		if(hObj.IsValid())
			hObj->Spawn();
	}
}
const pragma::physics::ICollisionObject *pragma::physics::PhysObj::GetCollisionObject() const { return const_cast<PhysObj *>(this)->GetCollisionObject(); }
std::vector<pragma::util::TSharedHandle<pragma::physics::ICollisionObject>> &pragma::physics::PhysObj::GetCollisionObjects() { return m_collisionObjects; }
const std::vector<pragma::util::TSharedHandle<pragma::physics::ICollisionObject>> &pragma::physics::PhysObj::GetCollisionObjects() const { return const_cast<PhysObj *>(this)->GetCollisionObjects(); }
pragma::physics::ICollisionObject *pragma::physics::PhysObj::GetCollisionObject()
{
	if(m_collisionObjects.empty())
		return nullptr;
	auto &hObj = m_collisionObjects.front();
	if(!hObj.IsValid())
		return nullptr;
	return hObj.Get();
}

pragma::physics::PhysObj::~PhysObj()
{
	InvalidateHandle();
	//auto *state = m_networkState;
	for(unsigned int i = 0; i < m_collisionObjects.size(); i++) {
		auto &o = m_collisionObjects[i];
		if(o.IsValid())
			o.Remove();
	}
}
void pragma::physics::PhysObj::OnSleep()
{
	if(m_owner.expired())
		return;
	auto pPhysComponent = m_owner->GetEntity().GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->OnPhysicsSleep(this);
}
void pragma::physics::PhysObj::OnWake()
{
	if(m_owner.expired())
		return;
	auto pPhysComponent = m_owner->GetEntity().GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->OnPhysicsWake(this);
}
void pragma::physics::PhysObj::Enable() { math::set_flag(m_stateFlags, StateFlags::Disabled, false); }
void pragma::physics::PhysObj::Disable() { math::set_flag(m_stateFlags, StateFlags::Disabled, true); }
pragma::BaseEntityComponent *pragma::physics::PhysObj::GetOwner() { return m_owner.get(); }
bool pragma::physics::PhysObj::IsController() const { return false; }

void pragma::physics::PhysObj::SetLinearFactor(const Vector3 &) {}
void pragma::physics::PhysObj::SetAngularFactor(const Vector3 &) {}
Vector3 pragma::physics::PhysObj::GetLinearFactor() const { return Vector3(0.f, 0.f, 0.f); }
Vector3 pragma::physics::PhysObj::GetAngularFactor() const { return Vector3(0.f, 0.f, 0.f); }

void pragma::physics::PhysObj::SetLinearSleepingThreshold(float threshold) { SetSleepingThresholds(threshold, GetAngularSleepingThreshold()); }
void pragma::physics::PhysObj::SetAngularSleepingThreshold(float threshold) { SetSleepingThresholds(GetLinearSleepingThreshold(), threshold); }
std::pair<float, float> pragma::physics::PhysObj::GetSleepingThreshold() const { return {GetLinearSleepingThreshold(), GetAngularSleepingThreshold()}; }
void pragma::physics::PhysObj::SetSleepingThresholds(float linear, float angular) {}
float pragma::physics::PhysObj::GetLinearSleepingThreshold() const { return 0.f; }
float pragma::physics::PhysObj::GetAngularSleepingThreshold() const { return 0.f; }

void pragma::physics::PhysObj::Simulate(double, bool) {}

PhysObjHandle pragma::physics::PhysObj::GetHandle() const { return BaseLuaHandle::GetHandle<PhysObj>(); }

void pragma::physics::PhysObj::SetPosition(const Vector3 &pos)
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
	for(unsigned int i = 0; i < m_collisionObjects.size(); i++) {
		auto &o = m_collisionObjects[i];
		if(o.IsValid()) {
			Vector3 posOther = o->GetPos();
			o->SetPos(r + (posOther - posRoot));
		}
	}
}
void pragma::physics::PhysObj::SetOrientation(const Quat &q)
{
	if(m_collisionObjects.empty())
		return;
	auto &root = m_collisionObjects.front();
	if(!root.IsValid())
		return;
	auto oldRot = root->GetRotation();
	auto origin = root->GetPos();

	auto rotOffset = uquat::get_inverse(oldRot) * q;
	for(auto it = m_collisionObjects.begin(); it != m_collisionObjects.end(); ++it) {
		auto &o = *it;
		if(o.IsValid() && o.Get() != root.Get()) {
			auto offset = o->GetPos() - origin;
			uvec::rotate(&offset, rotOffset);
			o->SetRotation(rotOffset * o->GetRotation());
			o->SetPos(origin + offset);
		}
	}
	root->SetRotation(q); // Faster and less prone to precision errors
}
Quat pragma::physics::PhysObj::GetOrientation() const
{
	auto *o = GetCollisionObject();
	if(o == nullptr)
		return uquat::identity();
	return o->GetRotation();
}
Vector3 pragma::physics::PhysObj::GetPosition() const
{
	auto *o = GetCollisionObject();
	if(o == nullptr)
		return Vector3(0, 0, 0);
	auto r = o->GetPos();
	//return r;
	if(!o->HasOrigin()) // If origin is enabled, position can be vastly off from the entity position (e.g. player ragdoll)
		return r;
	auto rot = o->GetRotation();
	auto &origin = o->GetOrigin();
	r += origin * uquat::get_inverse(rot);
	return r;
}
Vector3 pragma::physics::PhysObj::GetOrigin() const
{
	auto *o = const_cast<PhysObj *>(this)->GetCollisionObject();
	if(o == nullptr)
		return Vector3(0, 0, 0);
	auto r = o->GetPos();
	if(!o->HasOrigin()) // If origin is enabled, position can be vastly off from the entity position (e.g. player ragdoll)
		return r;
	auto rot = o->GetRotation();
	auto &origin = o->GetOrigin();
	r += origin * uquat::get_inverse(rot);
	return r;
}
uint32_t pragma::physics::PhysObj::GetNumberOfCollisionObjectsAwake() const { return m_colObjAwakeCount; }
void pragma::physics::PhysObj::SetCollisionFilter(CollisionMask filterGroup, CollisionMask filterMask)
{
	m_collisionFilterGroup = filterGroup;
	m_collisionFilterMask = filterMask;
	for(auto &hObj : m_collisionObjects) {
		if(hObj.IsValid()) {
			hObj->SetCollisionFilterGroup(filterGroup);
			hObj->SetCollisionFilterMask(filterMask);
		}
	}
}
void pragma::physics::PhysObj::SetCollisionFilterMask(CollisionMask filterMask) { SetCollisionFilter(m_collisionFilterGroup, filterMask); }
void pragma::physics::PhysObj::AddCollisionFilter(CollisionMask filter)
{
	CollisionMask filterGroup;
	CollisionMask filterMask;
	GetCollisionFilter(&filterGroup, &filterMask);
	SetCollisionFilter(filterGroup | filter, filterMask | filter);
}
void pragma::physics::PhysObj::RemoveCollisionFilter(CollisionMask filter)
{
	CollisionMask filterGroup;
	CollisionMask filterMask;
	GetCollisionFilter(&filterGroup, &filterMask);
	SetCollisionFilter(filterGroup & ~filter, filterMask & ~filter);
}
void pragma::physics::PhysObj::SetCollisionFilter(CollisionMask filterGroup) { SetCollisionFilter(filterGroup, filterGroup); }
pragma::physics::CollisionMask pragma::physics::PhysObj::GetCollisionFilter() const { return m_collisionFilterGroup; }
pragma::physics::CollisionMask pragma::physics::PhysObj::GetCollisionFilterMask() const { return m_collisionFilterMask; }
void pragma::physics::PhysObj::GetCollisionFilter(CollisionMask *filterGroup, CollisionMask *filterMask) const
{
	*filterGroup = m_collisionFilterGroup;
	*filterMask = m_collisionFilterMask;
}

bool pragma::physics::PhysObj::IsTrigger() const
{
	auto *colObj = GetCollisionObject();
	return colObj && colObj->IsTrigger();
}
void pragma::physics::PhysObj::SetTrigger(bool bTrigger)
{
	for(auto &hColObj : m_collisionObjects) {
		if(hColObj.IsExpired())
			continue;
		hColObj->SetTrigger(bTrigger);
	}
}

void pragma::physics::PhysObj::SetLinearVelocity(const Vector3 &) {}
void pragma::physics::PhysObj::AddLinearVelocity(const Vector3 &vel) { SetLinearVelocity(GetLinearVelocity() + vel); }
Vector3 pragma::physics::PhysObj::GetAngularVelocity() const { return Vector3(0, 0, 0); }
void pragma::physics::PhysObj::SetAngularVelocity(const Vector3 &) {}
void pragma::physics::PhysObj::AddAngularVelocity(const Vector3 &vel) { SetAngularVelocity(GetAngularVelocity() + vel); }
void pragma::physics::PhysObj::PutToSleep() {}
void pragma::physics::PhysObj::WakeUp() {}
bool pragma::physics::PhysObj::IsSleeping() const { return m_colObjAwakeCount == m_collisionObjects.size(); }
void pragma::physics::PhysObj::SetDamping(float, float) {}
void pragma::physics::PhysObj::SetLinearDamping(float) {}
void pragma::physics::PhysObj::SetAngularDamping(float) {}
float pragma::physics::PhysObj::GetLinearDamping() const { return 0.f; }
float pragma::physics::PhysObj::GetAngularDamping() const { return 0.f; }

void pragma::physics::PhysObj::ApplyForce(const Vector3 &) {}
void pragma::physics::PhysObj::ApplyForce(const Vector3 &, const Vector3 &) {}
void pragma::physics::PhysObj::ApplyImpulse(const Vector3 &) {}
void pragma::physics::PhysObj::ApplyImpulse(const Vector3 &, const Vector3 &) {}
void pragma::physics::PhysObj::ApplyTorque(const Vector3 &) {}
void pragma::physics::PhysObj::ApplyTorqueImpulse(const Vector3 &) {}
void pragma::physics::PhysObj::ClearForces() {}
Vector3 pragma::physics::PhysObj::GetTotalForce() const { return Vector3(0.f, 0.f, 0.f); }
Vector3 pragma::physics::PhysObj::GetTotalTorque() const { return Vector3(0.f, 0.f, 0.f); }

std::ostream &operator<<(std::ostream &out, const pragma::physics::PhysObj &o)
{
	out << "PhysObj";
	out << "[FilterGroup:" << magic_enum::enum_flags_name(o.GetCollisionFilter()) << "]";
	out << "[FilterMask:" << magic_enum::enum_flags_name(o.GetCollisionFilterMask()) << "]";
	out << "[ColObjs:" << o.GetCollisionObjects().size() << "]";
	out << "[ColObjsAwake:" << o.GetNumberOfCollisionObjectsAwake() << "]";
	out << "[Vel:" << o.GetLinearVelocity() << "]";
	out << "[AngVel:" << o.GetAngularVelocity() << "]";
	return out;
}

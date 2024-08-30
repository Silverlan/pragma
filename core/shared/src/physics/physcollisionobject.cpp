/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/collision_object.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/collisionmasks.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/physics/base_t.hpp"

const double EPSILON = 4.9406564584125e-2;

pragma::physics::ICollisionObject::ICollisionObject(IEnvironment &env, pragma::physics::IShape &shape) : IBase {env}, m_shape {std::static_pointer_cast<pragma::physics::IShape>(shape.shared_from_this())} {}

void pragma::physics::ICollisionObject::OnRemove()
{
	auto *physObj = GetPhysObj();
	if(physObj)
		physObj->OnCollisionObjectRemoved(*this);

	RemoveWorldObject();
	m_physEnv.RemoveCollisionObject(*this);
	IBase::OnRemove();
}

UInt32 pragma::physics::ICollisionObject::GetBoneID() const { return m_boneId; }
void pragma::physics::ICollisionObject::SetBoneID(UInt32 id) { m_boneId = id; }

void pragma::physics::ICollisionObject::UpdateSurfaceMaterial()
{
	auto *shape = m_shape.get();
	if(shape == nullptr)
		return;
	if(umath::is_flag_set(m_stateFlags, StateFlags::CustomSurfaceMaterial)) {
		shape->SetSurfaceMaterial(m_surfaceMaterial);
		return;
	}
	auto *surfMat = shape->GetSurfaceMaterial();
	m_surfaceMaterial = surfMat ? surfMat->GetIndex() : -1;
}

int pragma::physics::ICollisionObject::GetSurfaceMaterial() const { return m_surfaceMaterial; }
void pragma::physics::ICollisionObject::SetSurfaceMaterial(int id)
{
	umath::set_flag(m_stateFlags, StateFlags::CustomSurfaceMaterial, true);
	auto *shape = GetCollisionShape();
	if(shape)
		shape->SetSurfaceMaterial(id);
}

bool pragma::physics::ICollisionObject::IsRigid() const { return false; }
bool pragma::physics::ICollisionObject::IsGhost() const { return false; }
bool pragma::physics::ICollisionObject::IsSoftBody() const { return false; }
bool pragma::physics::ICollisionObject::IsAsleep() const { return !IsAwake(); }
bool pragma::physics::ICollisionObject::IsAwake() const { return umath::is_flag_set(m_stateFlags, StateFlags::Awake); }

void pragma::physics::ICollisionObject::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<ICollisionObject>(lua); }

const pragma::physics::IRigidBody *pragma::physics::ICollisionObject::GetRigidBody() const { return const_cast<ICollisionObject *>(this)->GetRigidBody(); }
const pragma::physics::ISoftBody *pragma::physics::ICollisionObject::GetSoftBody() const { return const_cast<ICollisionObject *>(this)->GetSoftBody(); }
const pragma::physics::IGhostObject *pragma::physics::ICollisionObject::GetGhostObject() const { return const_cast<ICollisionObject *>(this)->GetGhostObject(); }

void pragma::physics::ICollisionObject::InitializeLuaHandle(const util::TWeakSharedHandle<IBase> &handle)
{
	IBase::InitializeLuaHandle(handle);
	UpdateSurfaceMaterial();
}

void pragma::physics::ICollisionObject::DoSpawn() { IWorldObject::DoSpawn(); }

Bool pragma::physics::ICollisionObject::HasOrigin() const { return umath::is_flag_set(m_stateFlags, StateFlags::HasOrigin); }
Vector3 pragma::physics::ICollisionObject::GetGravity() const
{
	auto *physObj = GetPhysObj();
	auto *ent = physObj ? physObj->GetOwner() : nullptr;
	if(ent == nullptr)
		return Vector3 {};
	auto gravityComponent = ent->GetEntity().GetComponent<GravityComponent>();
	return gravityComponent.valid() ? gravityComponent->GetGravityForce() : Vector3 {};
}
void pragma::physics::ICollisionObject::SetOrigin(const Vector3 &origin)
{
	m_origin = origin;
	if(m_origin.x < EPSILON && m_origin.x > -EPSILON && m_origin.y < EPSILON && m_origin.y > -EPSILON && m_origin.z < EPSILON && m_origin.z > -EPSILON) {
		m_origin = Vector3(0.f, 0.f, 0.f);
		umath::set_flag(m_stateFlags, StateFlags::HasOrigin, false);
	}
	else
		umath::set_flag(m_stateFlags, StateFlags::HasOrigin, true);
}
const Vector3 &pragma::physics::ICollisionObject::GetOrigin() const { return m_origin; }

void pragma::physics::ICollisionObject::UpdateAABB() { umath::set_flag(m_stateFlags, StateFlags::UpdateAABB, true); }
bool pragma::physics::ICollisionObject::ShouldUpdateAABB() const { return umath::is_flag_set(m_stateFlags, StateFlags::UpdateAABB); }
void pragma::physics::ICollisionObject::ResetUpdateAABBFlag() { umath::set_flag(m_stateFlags, StateFlags::UpdateAABB, false); }

void pragma::physics::ICollisionObject::SetCollisionShape(pragma::physics::IShape *shape)
{
	m_shape = shape ? std::static_pointer_cast<pragma::physics::IShape>(shape->shared_from_this()) : nullptr;
	ApplyCollisionShape(shape);
	if(IsSpawned())
		AddWorldObject();
	UpdateSurfaceMaterial();
}

const pragma::physics::IShape *pragma::physics::ICollisionObject::GetCollisionShape() const { return const_cast<ICollisionObject *>(this)->GetCollisionShape(); }
pragma::physics::IShape *pragma::physics::ICollisionObject::GetCollisionShape() { return m_shape.get(); }

void pragma::physics::ICollisionObject::DisableSimulation() { SetSimulationEnabled(false); }
void pragma::physics::ICollisionObject::EnableSimulation() { SetSimulationEnabled(true); }
void pragma::physics::ICollisionObject::SetCollisionFilterGroup(CollisionMask group)
{
	m_collisionFilterGroup = group;
	DoSetCollisionFilterGroup(group);
}
CollisionMask pragma::physics::ICollisionObject::GetCollisionFilterGroup() const { return m_collisionFilterGroup; }
void pragma::physics::ICollisionObject::SetCollisionFilterMask(CollisionMask mask)
{
	m_collisionFilterMask = mask;
	DoSetCollisionFilterMask(mask);
}
CollisionMask pragma::physics::ICollisionObject::GetCollisionFilterMask() const { return m_collisionFilterMask; }

void pragma::physics::ICollisionObject::SetContactReportEnabled(bool reportEnabled) { return umath::set_flag(m_stateFlags, StateFlags::ContactReportEnabled, reportEnabled); }
bool pragma::physics::ICollisionObject::IsContactReportEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::ContactReportEnabled); }

void pragma::physics::ICollisionObject::PreSimulate() {}
void pragma::physics::ICollisionObject::PostSimulate() {}

void pragma::physics::ICollisionObject::OnContact(const ContactInfo &contactInfo) { m_physEnv.OnContact(contactInfo); }
void pragma::physics::ICollisionObject::OnStartTouch(ICollisionObject &other) { m_physEnv.OnStartTouch(*this, other); }
void pragma::physics::ICollisionObject::OnEndTouch(ICollisionObject &other) { m_physEnv.OnEndTouch(*this, other); }
void pragma::physics::ICollisionObject::OnWake()
{
	if(IsAwake())
		return;
	umath::set_flag(m_stateFlags, StateFlags::Awake);
	m_physEnv.OnWake(*this);

	auto *physObj = GetPhysObj();
	if(physObj)
		physObj->OnCollisionObjectWake(*this);
}
void pragma::physics::ICollisionObject::OnSleep()
{
	if(IsAlwaysAwake() == true || IsAsleep())
		return;
	umath::set_flag(m_stateFlags, StateFlags::Awake, false);
	m_physEnv.OnSleep(*this);

	auto *physObj = GetPhysObj();
	if(physObj)
		physObj->OnCollisionObjectSleep(*this);
}
void pragma::physics::ICollisionObject::SetAlwaysAwake(bool alwaysAwake)
{
	umath::set_flag(m_stateFlags, StateFlags::AlwaysAwake, alwaysAwake);
	if(alwaysAwake) {
		if(IsAsleep())
			OnWake();
	}
}
bool pragma::physics::ICollisionObject::IsAlwaysAwake() const { return umath::is_flag_set(m_stateFlags, StateFlags::AlwaysAwake); }

pragma::physics::IRigidBody *pragma::physics::ICollisionObject::GetRigidBody() { return nullptr; }
pragma::physics::ISoftBody *pragma::physics::ICollisionObject::GetSoftBody() { return nullptr; }
pragma::physics::IGhostObject *pragma::physics::ICollisionObject::GetGhostObject() { return nullptr; }

//////////////////////////

bool pragma::physics::IGhostObject::IsGhost() const { return true; }
pragma::physics::IGhostObject *pragma::physics::IGhostObject::GetGhostObject() { return this; }
void pragma::physics::IGhostObject::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IGhostObject>(lua); }

//////////////////////////

pragma::physics::IRigidBody::IRigidBody(IEnvironment &env, pragma::physics::IShape &shape) : ICollisionObject {env, shape} {}
bool pragma::physics::IRigidBody::IsRigid() const { return true; }
pragma::physics::IRigidBody *pragma::physics::IRigidBody::GetRigidBody() { return this; }
void pragma::physics::IRigidBody::SetDamping(float linDamping, float angDamping)
{
	SetLinearDamping(linDamping);
	SetAngularDamping(angDamping);
}
void pragma::physics::IRigidBody::SetSleepingThresholds(float linear, float angular)
{
	SetLinearSleepingThreshold(linear);
	SetAngularSleepingThreshold(angular);
}
std::pair<float, float> pragma::physics::IRigidBody::GetSleepingThreshold() const { return {GetLinearSleepingThreshold(), GetAngularSleepingThreshold()}; }
void pragma::physics::IRigidBody::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IRigidBody>(lua); }

//////////////////////////

pragma::physics::ISoftBody::ISoftBody(IEnvironment &env, pragma::physics::IShape &shape, const std::vector<uint16_t> &meshVertIndicesToPhysIndices) : ICollisionObject {env, shape} {}
ModelSubMesh *pragma::physics::ISoftBody::GetSubMesh() const { return m_subMesh.expired() ? nullptr : m_subMesh.lock().get(); }

bool pragma::physics::ISoftBody::IsSoftBody() const { return true; }

pragma::physics::ISoftBody *pragma::physics::ISoftBody::GetSoftBody() { return this; }
void pragma::physics::ISoftBody::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<ISoftBody>(lua); }

//////////////////////////

std::ostream &operator<<(std::ostream &out, const pragma::physics::ICollisionObject &o)
{
	out << "PhysColObj";
	out << "[Pos:" << o.GetPos() << "]";
	out << "[Ang:" << EulerAngles {o.GetRotation()} << "]";
	out << "[Awake:" << o.IsAwake() << "]";
	Vector3 min, max;
	o.GetAABB(min, max);
	out << "[Bounds:(" << min << ")(" << max << ")]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::physics::IGhostObject &o)
{
	operator<<(out, static_cast<const pragma::physics::ICollisionObject &>(o));
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::physics::IRigidBody &o)
{
	operator<<(out, static_cast<const pragma::physics::ICollisionObject &>(o));
	out << "PhysColObj";
	out << "[Pos:" << o.GetPos() << "]";
	out << "[Ang:" << EulerAngles {o.GetRotation()} << "]";
	out << "[Vel:" << o.GetLinearVelocity() << "]";
	out << "[AngVel:" << o.GetAngularVelocity() << "]";
	out << "[Awake:" << o.IsAwake() << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::physics::ISoftBody &o)
{
	operator<<(out, static_cast<const pragma::physics::ICollisionObject &>(o));
	return out;
}

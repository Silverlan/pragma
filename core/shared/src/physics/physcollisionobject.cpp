#include "stdafx_shared.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/collision_object.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/collisionmasks.h"
#include "pragma/model/modelmesh.h"

const double EPSILON = 4.9406564584125e-2;

#pragma optimize("",off)
pragma::physics::ICollisionObject::ICollisionObject(IEnvironment &env,pragma::physics::IShape &shape)
	: IBase{env},m_shape{std::static_pointer_cast<pragma::physics::IShape>(shape.shared_from_this())}
{}

void pragma::physics::ICollisionObject::OnRemove()
{
	IBase::OnRemove();
	m_physEnv.RemoveCollisionObject(*this);
}

UInt32 pragma::physics::ICollisionObject::GetBoneID() const {return m_boneId;}
void pragma::physics::ICollisionObject::SetBoneID(UInt32 id) {m_boneId = id;}

void pragma::physics::ICollisionObject::UpdateSurfaceMaterial()
{
	if(m_customSurfaceMaterial == true)
		return;
	auto *shape = m_shape.get();
	if(shape == nullptr || shape->IsConvexHull() == false)
		return;
	m_surfaceMaterial = shape->GetConvexHullShape()->GetSurfaceMaterial();
}

int pragma::physics::ICollisionObject::GetSurfaceMaterial() const {return m_surfaceMaterial;}
void pragma::physics::ICollisionObject::SetSurfaceMaterial(int id) {m_surfaceMaterial = id; m_customSurfaceMaterial = true;}

bool pragma::physics::ICollisionObject::IsRigid() const {return false;}
bool pragma::physics::ICollisionObject::IsGhost() const {return false;}
bool pragma::physics::ICollisionObject::IsSoftBody() const {return false;}

const pragma::physics::IRigidBody *pragma::physics::ICollisionObject::GetRigidBody() const {return const_cast<ICollisionObject*>(this)->GetRigidBody();}
const pragma::physics::ISoftBody *pragma::physics::ICollisionObject::GetSoftBody() const {return const_cast<ICollisionObject*>(this)->GetSoftBody();}
const pragma::physics::IGhostObject *pragma::physics::ICollisionObject::GetGhostObject() const {return const_cast<ICollisionObject*>(this)->GetGhostObject();}

void pragma::physics::ICollisionObject::Initialize()
{
	auto &nwState = m_physEnv.GetNetworkState();
	InitializeLuaObject(nwState.GetLuaState());

	UpdateSurfaceMaterial();
}

void pragma::physics::ICollisionObject::Spawn()
{
	if(m_bSpawned == true)
		return;
	m_bSpawned = true;
	SetCollisionShape(m_shape.get()); // Add it to the physics environment
}

Bool pragma::physics::ICollisionObject::HasOrigin() const {return m_bHasOrigin;}
void pragma::physics::ICollisionObject::SetOrigin(const Vector3 &origin)
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
Vector3 &pragma::physics::ICollisionObject::GetOrigin() {return m_origin;}

void pragma::physics::ICollisionObject::AddWorldObject()
{
	if(m_bSpawned == false)
		return;
	RemoveWorldObject();
	DoAddWorldObject();
}

void pragma::physics::ICollisionObject::UpdateAABB() {m_bUpdateAABB = true;}
bool pragma::physics::ICollisionObject::ShouldUpdateAABB() const {return m_bUpdateAABB;}
void pragma::physics::ICollisionObject::ResetUpdateAABBFlag() {m_bUpdateAABB = false;}

void pragma::physics::ICollisionObject::SetCollisionShape(pragma::physics::IShape *shape)
{
	m_shape = shape ? std::static_pointer_cast<pragma::physics::IShape>(shape->shared_from_this()) : nullptr;
	ApplyCollisionShape(shape);
	AddWorldObject();
}

const pragma::physics::IShape *pragma::physics::ICollisionObject::GetCollisionShape() const {return const_cast<ICollisionObject*>(this)->GetCollisionShape();}
pragma::physics::IShape *pragma::physics::ICollisionObject::GetCollisionShape() {return m_shape.get();}

void pragma::physics::ICollisionObject::DisableSimulation() {SetSimulationEnabled(false);}
void pragma::physics::ICollisionObject::EnableSimulation() {SetSimulationEnabled(true);}
void pragma::physics::ICollisionObject::SetCollisionFilterGroup(CollisionMask group)
{
	m_collisionFilterGroup = group;
	DoSetCollisionFilterGroup(group);
}
CollisionMask pragma::physics::ICollisionObject::GetCollisionFilterGroup() const {return m_collisionFilterGroup;}
void pragma::physics::ICollisionObject::SetCollisionFilterMask(CollisionMask mask)
{
	m_collisionFilterMask = mask;
	DoSetCollisionFilterMask(mask);
}
CollisionMask pragma::physics::ICollisionObject::GetCollisionFilterMask() const {return m_collisionFilterMask;}

void pragma::physics::ICollisionObject::PreSimulate() {}
void pragma::physics::ICollisionObject::PostSimulate() {}

pragma::physics::IRigidBody *pragma::physics::ICollisionObject::GetRigidBody() {return nullptr;}
pragma::physics::ISoftBody *pragma::physics::ICollisionObject::GetSoftBody() {return nullptr;}
pragma::physics::IGhostObject *pragma::physics::ICollisionObject::GetGhostObject() {return nullptr;}

//////////////////////////

bool pragma::physics::IGhostObject::IsGhost() const {return true;}
pragma::physics::IGhostObject *pragma::physics::IGhostObject::GetGhostObject() {return this;}

//////////////////////////

pragma::physics::IRigidBody::IRigidBody(IEnvironment &env,float mass,pragma::physics::IShape &shape,const Vector3 &localInertia)
	: ICollisionObject{env,shape}
{}
bool pragma::physics::IRigidBody::IsRigid() const {return true;}
pragma::physics::IRigidBody *pragma::physics::IRigidBody::GetRigidBody() {return this;}
void pragma::physics::IRigidBody::SetDamping(float linDamping,float angDamping)
{
	SetLinearDamping(linDamping);
	SetAngularDamping(angDamping);
}
void pragma::physics::IRigidBody::SetSleepingThresholds(float linear,float angular)
{
	SetLinearSleepingThreshold(linear);
	SetAngularSleepingThreshold(angular);
}
std::pair<float,float> pragma::physics::IRigidBody::GetSleepingThreshold() const {return {GetLinearSleepingThreshold(),GetAngularSleepingThreshold()};}

//////////////////////////

pragma::physics::ISoftBody::ISoftBody(IEnvironment &env,pragma::physics::IShape &shape,const std::vector<uint16_t> &meshVertIndicesToPhysIndices)
	: ICollisionObject{env,shape}
{}
ModelSubMesh *pragma::physics::ISoftBody::GetSubMesh() const {return m_subMesh.expired() ? nullptr : m_subMesh.lock().get();}

bool pragma::physics::ISoftBody::IsSoftBody() const {return true;}

pragma::physics::ISoftBody *pragma::physics::ISoftBody::GetSoftBody() {return this;}
#pragma optimize("",on)

#include "stdafx_shared.h"
#include "pragma/physics/phystransform.h"
#include "pragma/physics/physenvironment.h"

PhysTransform::PhysTransform(const btTransform &t)
	: m_transform(t)
{}

PhysTransform::PhysTransform()
	: PhysTransform(btTransform())
{
	SetIdentity();
}

PhysTransform PhysTransform::GetInverse() const
{
	return PhysTransform {
		m_transform.inverse()
	};
}

void PhysTransform::SetTransform(const btTransform &t) {m_transform = t;}
const btTransform &PhysTransform::GetTransform() const {return m_transform;}
void PhysTransform::GetTransform(btTransform **t) {*t = &m_transform;}

Vector3 PhysTransform::GetOrigin() const
{
	auto &p = m_transform.getOrigin();
	return Vector3(p.x() /PhysEnv::WORLD_SCALE,p.y() /PhysEnv::WORLD_SCALE,p.z() /PhysEnv::WORLD_SCALE);
}

Quat PhysTransform::GetRotation() const
{
	btQuaternion rot = m_transform.getRotation();
	return Quat(
		static_cast<float>(rot.w()),
		static_cast<float>(rot.x()),
		static_cast<float>(rot.y()),
		static_cast<float>(rot.z())
	);
}

void PhysTransform::SetOrigin(const Vector3 &origin)
{
	m_transform.setOrigin(btVector3(origin.x,origin.y,origin.z) *PhysEnv::WORLD_SCALE);
}

void PhysTransform::SetRotation(const Quat &rot) {m_transform.setRotation(btQuaternion(rot.x,rot.y,rot.z,rot.w));}

void PhysTransform::SetIdentity()
{
	m_transform.setIdentity();
}

const Mat3 &PhysTransform::GetBasis() const {return const_cast<PhysTransform*>(this)->GetBasis();}
Mat3 &PhysTransform::GetBasis() {return reinterpret_cast<Mat3&>(m_transform.getBasis());}
void PhysTransform::SetBasis(const Mat3 &m) {m_transform.setBasis(reinterpret_cast<const btMatrix3x3&>(m));}
PhysTransform PhysTransform::operator*(const PhysTransform &tOther) const
{
	auto r = PhysTransform(m_transform);
	return r *= tOther;
}
PhysTransform &PhysTransform::operator*=(const PhysTransform &tOther)
{
	m_transform *= tOther.m_transform;
	return *this;
}
Vector3 PhysTransform::operator()(const Vector3 &x) const
{
	auto r = m_transform(uvec::create_bt(x) *PhysEnv::WORLD_SCALE);
	return uvec::create(r /PhysEnv::WORLD_SCALE);
}
Vector3 PhysTransform::operator*(const Vector3 &x) const
{
	auto r = m_transform *(uvec::create_bt(x) *PhysEnv::WORLD_SCALE);
	return uvec::create(r /PhysEnv::WORLD_SCALE);
}
Quat PhysTransform::operator*(const Quat &q) const
{
	auto r = m_transform *btQuaternion(q.x,q.y,q.z,q.w);
	return Quat(
		static_cast<float>(r.w()),
		static_cast<float>(r.x()),
		static_cast<float>(r.y()),
		static_cast<float>(r.z())
	);
}

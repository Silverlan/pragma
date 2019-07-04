#include "stdafx_shared.h"
#include "pragma/physics/transform.hpp"
#include "pragma/physics/environment.hpp"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

pragma::physics::Transform::Transform()
{
	SetIdentity();
}

pragma::physics::Transform::Transform(const Mat4 &t)
{
	umat::decompose(t,m_translation,m_rotation);
}

pragma::physics::Transform::Transform(const Vector3 &translation,const Quat &rotation)
	: m_translation{translation},m_rotation{rotation}
{}

pragma::physics::Transform pragma::physics::Transform::GetInverse() const
{
	return Transform{-m_translation,uquat::get_inverse(m_rotation)};
}

const Vector3 &pragma::physics::Transform::GetOrigin() const {return m_translation;}

const Quat &pragma::physics::Transform::GetRotation() const {return m_rotation;}

Mat4 pragma::physics::Transform::ToMatrix() const
{
	auto m = glm::translate(Mat4{1.f},m_translation);
	m *= umat::create(m_rotation);
	return m;
}

void pragma::physics::Transform::SetOrigin(const Vector3 &origin) {m_translation = origin;}

void pragma::physics::Transform::SetRotation(const Quat &rot) {m_rotation = rot;}

void pragma::physics::Transform::SetIdentity()
{
	m_translation = {};
	m_rotation = uquat::identity();
}
pragma::physics::Transform pragma::physics::Transform::operator*(const Transform &tOther) const
{
	auto res = *this;
	uvec::rotate(&res.m_translation,tOther.m_rotation);
	res.m_rotation *= tOther.m_rotation;
	return res;
}
pragma::physics::Transform &pragma::physics::Transform::operator*=(const Transform &tOther)
{
	*this = *this *tOther;
	return *this;
}
Vector3 pragma::physics::Transform::operator*(const Vector3 &translation) const
{
	auto result = translation;
	uvec::rotate(&result,m_rotation);
	result += m_translation;
	return result;
}
Quat pragma::physics::Transform::operator*(const Quat &rot) const
{
	auto result = rot;
	result *= m_rotation;
	return result;
}

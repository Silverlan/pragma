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
	Transform result {-m_translation,uquat::get_inverse(m_rotation)};
	uvec::rotate(&result.m_translation,result.m_rotation);
	return result;
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
void pragma::physics::Transform::TranslateGlobal(const Vector3 &v) {m_translation += v;}
void pragma::physics::Transform::TranslateLocal(const Vector3 &v)
{
	auto vrot = v;
	uvec::rotate(&vrot,m_rotation);
	m_translation += vrot;
}
void pragma::physics::Transform::RotateGlobal(const Quat &rot)
{
	uvec::rotate(&m_translation,rot);
	m_rotation = rot *m_rotation;
}
void pragma::physics::Transform::RotateLocal(const Quat &rot) {m_rotation *= rot;}
pragma::physics::Transform pragma::physics::Transform::operator*(const Transform &tOther) const
{
	auto res = *this;
	res *= tOther;
	return res;
}

pragma::physics::Transform &pragma::physics::Transform::operator*=(const Transform &tOther)
{
	auto translation = tOther.m_translation;
	uvec::rotate(&translation,m_rotation);
	m_rotation *= tOther.m_rotation;
	m_translation += translation;
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
	return m_rotation *rot;
}

/////////////

pragma::physics::ScaledTransform::ScaledTransform(const Transform &t)
	: Transform{t}
{}
pragma::physics::ScaledTransform::ScaledTransform(const Vector3 &pos,const Quat &rot,const Vector3 &scale)
	: Transform{pos,rot},m_scale{scale}
{}
void pragma::physics::ScaledTransform::SetIdentity()
{
	Transform::SetIdentity();
	m_scale = {1.f,1.f,1.f};
}
const Vector3 &pragma::physics::ScaledTransform::GetScale() const {return m_scale;}
void pragma::physics::ScaledTransform::SetScale(const Vector3 &scale) {m_scale = scale;}
void pragma::physics::ScaledTransform::Scale(const Vector3 &scale) {m_scale *= scale;}
pragma::physics::Transform pragma::physics::ScaledTransform::GetInverse() const
{
	ScaledTransform inverse {Transform::GetInverse()};
	inverse.SetScale(GetScale());
	return inverse;
}
pragma::physics::ScaledTransform pragma::physics::ScaledTransform::operator*(const ScaledTransform &tOther) const
{
	auto res = *this;
	res *= tOther;
	return res;
}
pragma::physics::ScaledTransform pragma::physics::ScaledTransform::operator*(const Transform &tOther) const
{
	auto res = *this;
	res *= tOther;
	return res;
}
pragma::physics::ScaledTransform &pragma::physics::ScaledTransform::operator*=(const ScaledTransform &tOther)
{
	Transform::operator*=(tOther);
	m_scale *= tOther.m_scale;
	return *this;
}
pragma::physics::ScaledTransform &pragma::physics::ScaledTransform::operator*=(const Transform &tOther)
{
	Transform::operator*=(tOther);
	return *this;
}
Vector3 pragma::physics::ScaledTransform::operator*(const Vector3 &translation) const
{
	return Transform::operator*(translation);
}
Quat pragma::physics::ScaledTransform::operator*(const Quat &rot) const
{
	return Transform::operator*(rot);
}

Mat4 pragma::physics::ScaledTransform::ToMatrix() const
{
	return glm::scale(glm::mat4{1.f},m_scale) *Transform::ToMatrix();
}

/////////////

Vector3 operator*(const Vector3 &v,const pragma::physics::Transform &t)
{
	return (pragma::physics::Transform{v,uquat::identity()} *t).GetOrigin();
}
Vector3 &operator*=(Vector3 &v,const pragma::physics::Transform &t)
{
	v = operator*(v,t);
	return v;
}
Quat operator*(const Quat &v,const pragma::physics::Transform &t)
{
	return (pragma::physics::Transform{Vector3{},v} *t).GetOrigin();
}
Quat &operator*=(Quat &v,const pragma::physics::Transform &t)
{
	v = operator*(v,t);
	return v;
}

Vector3 operator*(const Vector3 &v,const pragma::physics::ScaledTransform &t)
{
	return operator*(v,static_cast<const pragma::physics::Transform&>(t));
}
Vector3 &operator*=(Vector3 &v,const pragma::physics::ScaledTransform &t)
{
	return operator*=(v,static_cast<const pragma::physics::Transform&>(t));
}
Quat operator*(const Quat &v,const pragma::physics::ScaledTransform &t)
{
	return operator*(v,static_cast<const pragma::physics::Transform&>(t));
}
Quat &operator*=(Quat &v,const pragma::physics::ScaledTransform &t)
{
	return operator*=(v,static_cast<const pragma::physics::Transform&>(t));
}

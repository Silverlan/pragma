#ifndef __PHYS_TRANSFORM_HPP__
#define __PHYS_TRANSFORM_HPP__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <mathutil/glmutil.h>
#include <mathutil/uquat.h>
#include <vector>

namespace pragma::physics
{
	class DLLNETWORK Transform
	{
	public:
		Transform();
		Transform(const Mat4 &t);
		Transform(const Vector3 &translation,const Quat &rotation);
		~Transform()=default;

		const Vector3 &GetOrigin() const;
		const Quat &GetRotation() const;
		void SetOrigin(const Vector3 &origin);
		void SetRotation(const Quat &rot);
		void SetIdentity();
		void TranslateGlobal(const Vector3 &v);
		void TranslateLocal(const Vector3 &v);
		void RotateGlobal(const Quat &rot);
		void RotateLocal(const Quat &rot);
		Transform GetInverse() const;
		Transform operator*(const Transform &tOther) const;
		Transform &operator*=(const Transform &tOther);
		Vector3 operator*(const Vector3 &translation) const;
		Quat operator*(const Quat &rot) const;

		Mat4 ToMatrix() const;
	private:
		Vector3 m_translation = {};
		Quat m_rotation = uquat::identity();
	};

	class DLLNETWORK ScaledTransform
		: public Transform
	{
	public:
		using Transform::Transform;
		ScaledTransform(const Transform &t);
		ScaledTransform(const Vector3 &pos,const Quat &rot,const Vector3 &scale);
		void SetIdentity();
		const Vector3 &GetScale() const;
		void SetScale(const Vector3 &scale);
		void Scale(const Vector3 &scale);
		Transform GetInverse() const;
		ScaledTransform operator*(const ScaledTransform &tOther) const;
		ScaledTransform operator*(const Transform &tOther) const;
		ScaledTransform &operator*=(const ScaledTransform &tOther);
		ScaledTransform &operator*=(const Transform &tOther);
		Vector3 operator*(const Vector3 &translation) const;
		Quat operator*(const Quat &rot) const;

		Mat4 ToMatrix() const;
	private:
		Vector3 m_scale = {1.f,1.f,1.f};
	};
};

#endif

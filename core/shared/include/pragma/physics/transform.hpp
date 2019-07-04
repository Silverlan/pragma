#ifndef __PHYS_TRANSFORM_HPP__
#define __PHYS_TRANSFORM_HPP__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <pragma/physics/physapi.h>
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
};

#endif

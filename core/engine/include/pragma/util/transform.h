#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "pragma/definitions.h"
#include <mathutil/glmutil.h>

class DLLENGINE Transform
{
protected:
	Vector3 m_pos = {};
	Vector3 m_scale = {1.f,1.f,1.f};
	Quat m_orientation = uquat::identity();
	Mat4 m_transform = umat::identity();
public:
	Transform();
	Transform(const Vector3 &position);
	Transform(const Quat &orientation);
	Transform(const Vector3 &position,const Quat &orientation);
	const Vector3 &GetScale() const;
	const Vector3 &GetPosition() const;
	const Quat &GetOrientation() const;
	const Mat4 &GetTransformationMatrix() const;
	void SetScale(const Vector3 &scale);
	void SetPosition(const Vector3 &pos);
	void SetOrientation(const Quat &orientation);
	void UpdateMatrix();
};

#endif
#ifndef __PHYSTRANSFORM_H__
#define __PHYSTRANSFORM_H__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <pragma/physics/physapi.h>
#include <mathutil/glmutil.h>
#include <mathutil/uquat.h>
#include <vector>

class DLLNETWORK PhysTransform
{
private:
	btTransform m_transform = {};
public:
	PhysTransform();
	PhysTransform(const btTransform &t);
	Vector3 GetOrigin() const;
	Quat GetRotation() const;
	void SetOrigin(const Vector3 &origin);
	void SetRotation(const Quat &rot);
	void GetTransform(btTransform **t);
	const btTransform &GetTransform() const;
	void SetTransform(const btTransform &t);
	void SetIdentity();
	const Mat3 &GetBasis() const;
	Mat3 &GetBasis();
	void SetBasis(const Mat3 &m);
	PhysTransform GetInverse() const;
	PhysTransform operator*(const PhysTransform &tOther) const;
	PhysTransform &operator*=(const PhysTransform &tOther);
	Vector3 operator()(const Vector3 &x) const;
	Vector3 operator*(const Vector3 &x) const;
	Quat operator*(const Quat &q) const;
};

#endif
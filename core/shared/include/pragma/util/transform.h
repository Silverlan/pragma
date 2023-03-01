/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "pragma/definitions.h"
#include <mathutil/glmutil.h>

// Deprecated, replaced by umath::Transform
// TODO: Remove all instances of this, then remove this file!
class DLLNETWORK Transform {
  protected:
	Vector3 m_pos = {};
	Vector3 m_scale = {1.f, 1.f, 1.f};
	Quat m_orientation = uquat::identity();
	Mat4 m_transform = umat::identity();
  public:
	Transform();
	Transform(const Vector3 &position);
	Transform(const Quat &orientation);
	Transform(const Vector3 &position, const Quat &orientation);
	const Vector3 &GetScale() const;
	const Vector3 &GetPosition() const;
	const Quat &GetOrientation() const;
	const Mat4 &GetTransformationMatrix() const;
	void SetScale(const Vector3 &scale);
	void SetPosition(const Vector3 &pos);
	void SetOrientation(const Quat &orientation);
	void UpdateMatrix();
};

DLLNETWORK std::ostream &operator<<(std::ostream &out, const Transform &o);

#endif

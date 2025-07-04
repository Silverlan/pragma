// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ORIENTED_POINT_H__
#define __ORIENTED_POINT_H__

#include "pragma/definitions.h"
#include <mathutil/glmutil.h>

class DLLNETWORK OrientedPoint {
  public:
	OrientedPoint();
	OrientedPoint(Vector3 &position);
	OrientedPoint(Quat &orientation);
	OrientedPoint(Vector3 &position, Quat &orientation);
	Vector3 pos = {};
	Quat rot = uquat::identity();
};

#endif

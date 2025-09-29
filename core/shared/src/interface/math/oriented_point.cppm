// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include "mathutil/uquat.h"
#include "mathutil/uvec.h"


export module pragma.shared:math.oriented_point;

export class DLLNETWORK OrientedPoint {
  public:
	OrientedPoint();
	OrientedPoint(Vector3 &position);
	OrientedPoint(Quat &orientation);
	OrientedPoint(Vector3 &position, Quat &orientation);
	Vector3 pos = {};
	Quat rot = uquat::identity();
};

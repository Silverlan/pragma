/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ORIENTED_POINT_H__
#define __ORIENTED_POINT_H__

#include "pragma/definitions.h"
#include <mathutil/glmutil.h>

class DLLNETWORK OrientedPoint
{
public:
	OrientedPoint();
	OrientedPoint(Vector3 &position);
	OrientedPoint(Quat &orientation);
	OrientedPoint(Vector3 &position,Quat &orientation);
	Vector3 pos = {};
	Quat rot = uquat::identity();
};

#endif
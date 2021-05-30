/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/orientedpoint.h"

OrientedPoint::OrientedPoint()
{}
OrientedPoint::OrientedPoint(Vector3 &position)
	: pos(position)
{}
OrientedPoint::OrientedPoint(Quat &orientation)
	: rot(orientation)
{}
OrientedPoint::OrientedPoint(Vector3 &position,Quat &orientation)
	: pos(position),rot(orientation)
{}

// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/uvec.h"

module pragma.shared;

import :math.oriented_point;

OrientedPoint::OrientedPoint() {}
OrientedPoint::OrientedPoint(Vector3 &position) : pos(position) {}
OrientedPoint::OrientedPoint(Quat &orientation) : rot(orientation) {}
OrientedPoint::OrientedPoint(Vector3 &position, Quat &orientation) : pos(position), rot(orientation) {}

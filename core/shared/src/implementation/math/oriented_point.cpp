// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math.oriented_point;

pragma::math::OrientedPoint::OrientedPoint() {}
pragma::math::OrientedPoint::OrientedPoint(Vector3 &position) : pos(position) {}
pragma::math::OrientedPoint::OrientedPoint(Quat &orientation) : rot(orientation) {}
pragma::math::OrientedPoint::OrientedPoint(Vector3 &position, Quat &orientation) : pos(position), rot(orientation) {}

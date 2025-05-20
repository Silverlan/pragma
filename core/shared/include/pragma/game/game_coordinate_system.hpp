/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GAME_COORDINATE_SYSTEM_HPP__
#define __GAME_COORDINATE_SYSTEM_HPP__

#include <cinttypes>
#include <mathutil/uvec.h>

namespace pragma {
	enum class Axis : uint8_t { X = 0u, Y, Z, Count };
	enum class SignedAxis : uint8_t { X = 0u, Y, Z, NegX, NegY, NegZ, Count };
	enum class RotationOrder : uint16_t { XYZ = 0u, YXZ, XZX, XYX, YXY, YZY, ZYZ, ZXZ, XZY, YZX, ZYX, ZXY };
	constexpr bool is_positive_axis(Axis axis) { return true; }
	constexpr bool is_negative_axis(Axis axis) { return false; }
	constexpr bool is_positive_axis(SignedAxis axis) { return (umath::to_integral(axis) < umath::to_integral(SignedAxis::NegX)); }
	constexpr bool is_negative_axis(SignedAxis axis) { return !is_positive_axis(axis); }

	constexpr Vector3 axis_to_vector(SignedAxis axis)
	{
		switch(axis) {
		case pragma::SignedAxis::X:
			return Vector3 {1.f, 0.f, 0.f};
		case pragma::SignedAxis::Y:
			return Vector3 {0.f, 1.f, 0.f};
		case pragma::SignedAxis::Z:
			return Vector3 {0.f, 0.f, 1.f};
		case pragma::SignedAxis::NegX:
			return Vector3 {-1.f, 0.f, 0.f};
		case pragma::SignedAxis::NegY:
			return Vector3 {0.f, -1.f, 0.f};
		case pragma::SignedAxis::NegZ:
			return Vector3 {0.f, 0.f, -1.f};
		default:
			break;
		}
		return Vector3 {};
	}
	constexpr Vector3 axis_to_vector(Axis axis) { return axis_to_vector(static_cast<SignedAxis>(axis)); }
};

constexpr pragma::SignedAxis operator+(pragma::SignedAxis axis)
{
	switch(axis) {
	case pragma::SignedAxis::NegX:
		return pragma::SignedAxis::X;
	case pragma::SignedAxis::NegY:
		return pragma::SignedAxis::Y;
	case pragma::SignedAxis::NegZ:
		return pragma::SignedAxis::Z;
	default:
		return axis;
	}
}
constexpr pragma::SignedAxis operator-(pragma::SignedAxis axis)
{
	switch(axis) {
	case pragma::SignedAxis::X:
		return pragma::SignedAxis::NegX;
	case pragma::SignedAxis::Y:
		return pragma::SignedAxis::NegY;
	case pragma::SignedAxis::Z:
		return pragma::SignedAxis::NegZ;
	default:
		return axis;
	}
}

#endif

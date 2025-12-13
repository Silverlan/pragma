// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:game.coordinate_system;

export import pragma.math;

export {
	namespace pragma {
		enum class Axis : uint8_t { X = 0u, Y, Z, Count };
		enum class SignedAxis : uint8_t { X = 0u, Y, Z, NegX, NegY, NegZ, Count };
		enum class RotationOrder : uint16_t { XYZ = 0u, YXZ, XZX, XYX, YXY, YZY, ZYZ, ZXZ, XZY, YZX, ZYX, ZXY };
		constexpr bool is_positive_axis(Axis axis) { return true; }
		constexpr bool is_negative_axis(Axis axis) { return false; }
		constexpr bool is_positive_axis(SignedAxis axis) { return (math::to_integral(axis) < math::to_integral(SignedAxis::NegX)); }
		constexpr bool is_negative_axis(SignedAxis axis) { return !is_positive_axis(axis); }

		constexpr Vector3 axis_to_vector(SignedAxis axis)
		{
			switch(axis) {
			case SignedAxis::X:
				return Vector3 {1.f, 0.f, 0.f};
			case SignedAxis::Y:
				return Vector3 {0.f, 1.f, 0.f};
			case SignedAxis::Z:
				return Vector3 {0.f, 0.f, 1.f};
			case SignedAxis::NegX:
				return Vector3 {-1.f, 0.f, 0.f};
			case SignedAxis::NegY:
				return Vector3 {0.f, -1.f, 0.f};
			case SignedAxis::NegZ:
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
};

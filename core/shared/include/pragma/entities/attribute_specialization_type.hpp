// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ATTRIBUTE_SPECIALIZATION_TYPES_HPP__
#define __ATTRIBUTE_SPECIALIZATION_TYPES_HPP__

#include <cinttypes>

namespace pragma {
	enum class AttributeSpecializationType : uint8_t {
		None = 0,
		Color = 1,
		Distance,
		LightIntensity,
		Angle,
		File,
		Directory,
		Plane,
		Normal,
		Fraction,

		Custom,

		Count
	};
};

#endif

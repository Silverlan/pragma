/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ATTRIBUTE_SPECIALIZATION_TYPES_HPP__
#define __ATTRIBUTE_SPECIALIZATION_TYPES_HPP__

#include <cinttypes>

namespace pragma
{
	enum class AttributeSpecializationType : uint8_t
	{
		None = 0,
		Color = 1,
		Distance,
		LightIntensity,
		Angle,
		File,
		Directory,
		Plane,
		Normal,

		Custom,

		Count
	};
};

#endif

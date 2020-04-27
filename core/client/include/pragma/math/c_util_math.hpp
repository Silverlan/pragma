/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_UTIL_MATH_HPP__
#define __C_UTIL_MATH_HPP__

#include "pragma/clientdefinitions.h"
#include <mathutil/umat.h>
#include <array>

enum class LightType : uint8_t;
namespace pragma::math
{
	DLLCLIENT const std::array<Mat4,6> &get_cubemap_view_matrices();
	DLLCLIENT const Mat4 &get_cubemap_projection_matrix(float aspectRatio=1.f,float nearZ=0.1f,float farZ=10.f);

	// Converts a candela light intensity value to watts.
	// Note that the conversion is arbitrary and is not based on
	// a conventional formula, as conventional conversion attempts
	// have not resulted in good looking results.
	DLLCLIENT float light_intensity_to_watts(float candelaOrLux,LightType lightType);
};

#endif

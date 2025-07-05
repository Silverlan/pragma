// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_UTIL_MATH_HPP__
#define __C_UTIL_MATH_HPP__

#include "pragma/clientdefinitions.h"
#include <mathutil/umath_lighting.hpp>
#include <mathutil/umat.h>
#include <array>

enum class LightType : uint8_t;
namespace pragma {
	class CLightComponent;
};
namespace pragma::math {
	DLLCLIENT const std::array<Mat4, 6> &get_cubemap_view_matrices();
	DLLCLIENT const Mat4 &get_cubemap_projection_matrix(float aspectRatio = 1.f, float nearZ = 0.1f, float farZ = 10.f);

	namespace cycles {
		Watt get_light_power(const CLightComponent &light);
	};
};

#endif

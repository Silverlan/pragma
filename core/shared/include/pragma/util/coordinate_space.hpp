// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_UTIL_COORDINATE_SPACE_HPP__
#define __PRAGMA_UTIL_COORDINATE_SPACE_HPP__

#include <mathutil/umath.h>

namespace pragma {
	enum class CoordinateSpace : uint8_t {
		World = umath::to_integral(umath::CoordinateSpace::World),
		Local = umath::to_integral(umath::CoordinateSpace::Local),
		Object = umath::to_integral(umath::CoordinateSpace::Object),
	};
};

#endif

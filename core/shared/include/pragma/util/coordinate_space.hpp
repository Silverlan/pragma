/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

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

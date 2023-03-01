/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __E_FRUSTUM_H__
#define __E_FRUSTUM_H__

#include <cinttypes>

#undef NEAR
#undef FAR
enum class FrustumPlane : uint32_t {
	Left,
	Right,
	Top,
	Bottom,
	Near,
	Far,

	Count
};

enum class FrustumPoint : uint32_t { FarBottomLeft = 0, FarTopLeft, FarTopRight, FarBottomRight, NearBottomLeft, NearTopLeft, NearTopRight, NearBottomRight };

#endif

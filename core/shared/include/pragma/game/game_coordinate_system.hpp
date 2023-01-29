/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GAME_COORDINATE_SYSTEM_HPP__
#define __GAME_COORDINATE_SYSTEM_HPP__

#include <cinttypes>

namespace pragma {
	enum class Axis : uint8_t { X = 0u, Y, Z };
	enum class RotationOrder : uint16_t { XYZ = 0u, YXZ, XZX, XYX, YXY, YZY, ZYZ, ZXZ, XZY, YZX, ZYX, ZXY };
};

#endif

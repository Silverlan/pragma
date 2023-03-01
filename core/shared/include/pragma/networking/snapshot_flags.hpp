/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SNAPSHOT_FLAGS_HPP__
#define __SNAPSHOT_FLAGS_HPP__

#include <mathutil/umath.h>

namespace pragma {
	enum class SnapshotFlags : uint8_t { None = 0u, PhysicsData = 1u, ComponentData = PhysicsData << 1u };
	REGISTER_BASIC_BITWISE_OPERATORS(SnapshotFlags);
};

#endif

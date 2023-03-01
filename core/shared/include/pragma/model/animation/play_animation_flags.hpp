/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PLAY_ANIMATION_FLAGS_HPP__
#define __PLAY_ANIMATION_FLAGS_HPP__

#include <cinttypes>
#include <mathutil/umath.h>

namespace pragma {
	enum class FPlayAnim : uint32_t {
		None = 0u,
		Reset = 1u,
		Transmit = Reset << 1u,  // Only used for entities
		SnapTo = Transmit << 1u, // Unused
		Loop = SnapTo << 1u,

		Default = Transmit
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::FPlayAnim)

#endif

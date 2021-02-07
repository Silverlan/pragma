/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __KEY_STATE_HPP__
#define __KEY_STATE_HPP__

#include "pragma/definitions.h"
#include <cinttypes>

// These have to correspond to GLFW::KeyState!
enum class DLLNETWORK KeyState : uint32_t
{
	Invalid = std::numeric_limits<std::underlying_type_t<KeyState>>::max(),
	Press = 1,
	Release = 0,
	Repeat = 2,
	Held = Repeat +1
};

#endif

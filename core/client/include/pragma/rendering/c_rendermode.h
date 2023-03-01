/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDERMODE_H__
#define __C_RENDERMODE_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/umath.h>
#include <sharedutils/magic_enum.hpp>
#include <cinttypes>
#include <stdexcept>

namespace pragma::rendering {
	enum class SceneRenderPass : uint8_t { None = 0, World, View, Sky, Count };

	enum class RenderMask : uint64_t { None = 0u };
	using RenderGroup = RenderMask;
};
REGISTER_BASIC_BITWISE_OPERATORS_2WAY(pragma::rendering::RenderMask)

#endif

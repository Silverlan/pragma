// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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

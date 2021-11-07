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

namespace pragma::rendering
{
	enum class SceneRenderPass : uint8_t
	{
		None = 0,
		World,
		View,
		Skybox,
		Water,
		Count
	};

	enum class RenderMask : uint64_t
	{
		None = 0u,
		WorldBit = 1u,
		ViewBit = WorldBit<<1u,
		SkyBit = ViewBit<<1u,
		WaterBit = SkyBit<<1u,
		DepthPrepassBit = WaterBit<<1u,

		CustomStartBit = DepthPrepassBit<<1u,
		AnyScene = WorldBit | ViewBit | SkyBit | WaterBit | DepthPrepassBit
	};
	using RenderGroup = RenderMask;

	constexpr RenderMask scene_render_pass_to_render_mask(SceneRenderPass pass)
	{
		switch(pass)
		{
		case SceneRenderPass::World:
			return RenderMask::WorldBit;
		case SceneRenderPass::View:
			return RenderMask::ViewBit;
		case SceneRenderPass::Skybox:
			return RenderMask::SkyBit;
		case SceneRenderPass::Water:
			return RenderMask::WaterBit;
		case SceneRenderPass::None:
			return RenderMask::None;
		}
		throw std::runtime_error{"Invalid scene render pass " +std::string{magic_enum::enum_name(pass)}};
	}
};
REGISTER_BASIC_BITWISE_OPERATORS_2WAY(pragma::rendering::RenderMask)

#endif
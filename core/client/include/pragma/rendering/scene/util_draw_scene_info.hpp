/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_DRAW_SCENE_INFO_HPP__
#define __UTIL_DRAW_SCENE_INFO_HPP__

#include "pragma/rendering/c_renderflags.h"
#include "pragma/rendering/c_settings.hpp"
#include "pragma/rendering/render_stats.hpp"
#include <memory>
#include <optional>
#include <functional>
#include <mathutil/color.h>

class CBaseEntity;
namespace prosper {class IPrimaryCommandBuffer;};
namespace pragma {class CSceneComponent;};
namespace util
{
	struct DLLCLIENT DrawSceneInfo
	{
		enum class Flags : uint8_t
		{
			None = 0u,
			FlipVertically = 1u,
			DisableRender = FlipVertically<<1u
		};
		DrawSceneInfo()=default;
		DrawSceneInfo(const DrawSceneInfo &other)
			: scene{other.scene},commandBuffer{other.commandBuffer},renderTarget{other.renderTarget},
			renderFlags{other.renderFlags},clearColor{other.clearColor},toneMapping{other.toneMapping},
			prepassFilter{other.prepassFilter},renderFilter{other.renderFilter},outputImage{other.outputImage},
			outputLayerId{other.outputLayerId},flags{other.flags},renderStats{other.renderStats ? std::make_unique<RenderStats>(*other.renderStats) : nullptr}
		{}
		DrawSceneInfo &operator=(const DrawSceneInfo &other)
		{
			scene = other.scene;
			commandBuffer = other.commandBuffer;
			renderTarget = other.renderTarget;
			renderFlags = other.renderFlags;
			clearColor = other.clearColor;
			toneMapping = other.toneMapping;

			prepassFilter = other.prepassFilter;
			renderFilter = other.renderFilter;
			outputImage = other.outputImage;

			outputLayerId = other.outputLayerId;
			flags = other.flags;
			renderStats = other.renderStats ? std::make_unique<RenderStats>(*other.renderStats) : nullptr;
			return *this;
		}
		util::WeakHandle<::pragma::CSceneComponent> scene = {};
		mutable std::shared_ptr<prosper::IPrimaryCommandBuffer> commandBuffer = nullptr;
		std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
		FRender renderFlags = FRender::All;
		std::optional<Color> clearColor = {};
		std::optional<::pragma::rendering::ToneMapping> toneMapping {};
		
		std::function<bool(CBaseEntity&)> prepassFilter = nullptr;
		std::function<bool(CBaseEntity&)> renderFilter = nullptr;

		std::shared_ptr<prosper::IImage> outputImage = nullptr;
		uint32_t outputLayerId = 0u;
		Flags flags = Flags::None;

		mutable std::unique_ptr<RenderStats> renderStats = nullptr;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(util::DrawSceneInfo::Flags)

#endif

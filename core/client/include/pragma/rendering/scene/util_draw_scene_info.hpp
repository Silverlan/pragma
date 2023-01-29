/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __UTIL_DRAW_SCENE_INFO_HPP__
#define __UTIL_DRAW_SCENE_INFO_HPP__

#include "pragma/rendering/c_renderflags.h"
#include "pragma/rendering/c_settings.hpp"
#include "pragma/rendering/render_stats.hpp"
#include "pragma/rendering/c_rendermode.h"
#include <sharedutils/util_shared_handle.hpp>
#include <memory>
#include <optional>
#include <functional>
#include <mathutil/color.h>

class CBaseEntity;
class CGame;
namespace prosper {
	class IPrimaryCommandBuffer;
	class ICommandBuffer;
	class RenderTarget;
	class IImage;
};
namespace pragma {
	class CSceneComponent;
};
namespace util {
	struct DLLCLIENT DrawSceneInfo {
		enum class Flags : uint8_t { None = 0u, FlipVertically = 1u, DisableRender = FlipVertically << 1u, Reflection = DisableRender << 1u, DisablePrepass = Reflection << 1u, DisableLightingPass = DisablePrepass << 1u };
		DrawSceneInfo();
		DrawSceneInfo(const DrawSceneInfo &other);
		DrawSceneInfo &operator=(const DrawSceneInfo &other);
		util::TWeakSharedHandle<::pragma::CSceneComponent> scene = util::TWeakSharedHandle<::pragma::CSceneComponent> {};
		mutable std::shared_ptr<prosper::IPrimaryCommandBuffer> commandBuffer = nullptr;
		std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
		RenderFlags renderFlags = RenderFlags::All;
		std::optional<Color> clearColor = {};
		std::optional<::pragma::rendering::ToneMapping> toneMapping {};
		std::optional<Vector4> clipPlane {};
		std::optional<Vector3> pvsOrigin {};
		::pragma::rendering::RenderMask exclusionMask = ::pragma::rendering::RenderMask::None;
		::pragma::rendering::RenderMask inclusionMask = ::pragma::rendering::RenderMask::None;

		std::function<bool(CBaseEntity &)> prepassFilter = nullptr;
		std::function<bool(CBaseEntity &)> renderFilter = nullptr;

		std::shared_ptr<prosper::IImage> outputImage = nullptr;
		uint32_t outputLayerId = 0u;
		Flags flags = Flags::None;

		mutable std::unique_ptr<RenderStats> renderStats = nullptr;
		std::unique_ptr<std::vector<DrawSceneInfo>> subPasses = nullptr;

		::pragma::rendering::RenderMask GetRenderMask(CGame &game) const;
		Vector3 GetPvsOrigin() const;
		void AddSubPass(const DrawSceneInfo &drawSceneInfo);
		const std::vector<DrawSceneInfo> *GetSubPasses() const;
	};
	struct DLLCLIENT RenderPassDrawInfo {
		RenderPassDrawInfo(const DrawSceneInfo &drawSceneInfo, prosper::ICommandBuffer &cmdBuffer);
		const DrawSceneInfo &drawSceneInfo;
		mutable std::shared_ptr<prosper::ICommandBuffer> commandBuffer = nullptr;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(util::DrawSceneInfo::Flags)

#endif

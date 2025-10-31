// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "prosper_command_buffer.hpp"
#include <mathutil/color.h>

export module pragma.client:rendering.draw_scene_info;

import :rendering.enums;
import :rendering.render_stats;

export {
	class CBaseEntity;
	namespace pragma {
		class CSceneComponent;
	};
};
export namespace util {
	struct DLLCLIENT DrawSceneInfo {
		enum class Flags : uint8_t {
			None = 0u,
			FlipVertically = 1u,
			DisableRender = FlipVertically << 1u,
			Reflection = DisableRender << 1u,
			DisablePrepass = Reflection << 1u,
			DisableLightingPass = DisablePrepass << 1u,
			DisableGlowPass = DisableLightingPass << 1u,
		};
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

		::pragma::rendering::RenderMask GetRenderMask(pragma::Game &game) const;
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
export {
	REGISTER_BASIC_BITWISE_OPERATORS(util::DrawSceneInfo::Flags)
};

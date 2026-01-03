// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.draw_scene_info;

export import :rendering.enums;
import :rendering.render_stats;

export {
	namespace pragma {
		class CSceneComponent;
		namespace ecs {
			class CBaseEntity;
		}
	};
};
export namespace pragma::rendering {
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
		util::TWeakSharedHandle<CSceneComponent> scene = pragma::util::TWeakSharedHandle<CSceneComponent> {};
		mutable std::shared_ptr<prosper::IPrimaryCommandBuffer> commandBuffer = nullptr;
		std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
		RenderFlags renderFlags = RenderFlags::All;
		std::optional<Color> clearColor = {};
		std::optional<ToneMapping> toneMapping {};
		std::optional<Vector4> clipPlane {};
		std::optional<Vector3> pvsOrigin {};
		RenderMask exclusionMask = RenderMask::None;
		RenderMask inclusionMask = RenderMask::None;

		std::function<bool(ecs::CBaseEntity &)> prepassFilter = nullptr;
		std::function<bool(ecs::CBaseEntity &)> renderFilter = nullptr;

		std::shared_ptr<prosper::IImage> outputImage = nullptr;
		uint32_t outputLayerId = 0u;
		Flags flags = Flags::None;

		mutable std::unique_ptr<RenderStats> renderStats = nullptr;
		std::unique_ptr<std::vector<DrawSceneInfo>> subPasses = nullptr;

		RenderMask GetRenderMask(Game &game) const;
		Vector3 GetPvsOrigin() const;
		void AddSubPass(const DrawSceneInfo &drawSceneInfo);
		const std::vector<DrawSceneInfo> *GetSubPasses() const;
	};
	struct DLLCLIENT RenderPassDrawInfo {
		RenderPassDrawInfo(const DrawSceneInfo &drawSceneInfo, prosper::ICommandBuffer &cmdBuffer);
		const DrawSceneInfo &drawSceneInfo;
		mutable std::shared_ptr<prosper::ICommandBuffer> commandBuffer = nullptr;
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::rendering::DrawSceneInfo::Flags)
}

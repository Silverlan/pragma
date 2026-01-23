// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.cycles;

export import pragma.shared;

export namespace pragma {
	class ClientState;
}
export namespace pragma::rendering::cycles {
	struct DLLCLIENT SceneInfo {
		enum class DeviceType : uint8_t { CPU = 0, GPU };
		enum class SceneFlags : uint8_t // Has to match struct defined in Cycles module!
		{
			None = 0u,
			CullObjectsOutsidePvs = 1u,
			CullObjectsOutsideCameraFrustum = CullObjectsOutsidePvs << 1u
		};
		struct DLLCLIENT ColorTransform {
			std::string config;
			std::string look;
		};
		uint32_t width = 1'024;
		uint32_t height = 768;
		uint32_t samples = 1'024;
		bool denoise = true;
		bool hdrOutput = false;
		bool renderJob = false;
		float globalLightIntensityFactor = 1.f;
		std::string renderer = "cycles";
		SceneFlags sceneFlags = static_cast<SceneFlags>(math::to_integral(SceneFlags::CullObjectsOutsidePvs) | math::to_integral(SceneFlags::CullObjectsOutsideCameraFrustum));
		DeviceType device = DeviceType::CPU;
		std::string sky = "";
		float skyStrength = 1.f;
		EulerAngles skyAngles = {};
		uint32_t maxTransparencyBounces = 64;
		float exposure = 1.f;
		std::optional<ColorTransform> colorTransform {};
	};
	struct DLLCLIENT RenderImageInfo {
		math::Transform camPose {};
		bool equirectPanorama = false;
		Mat4 viewProjectionMatrix = {};
		float nearZ = baseEnvCameraComponent::DEFAULT_NEAR_Z;
		float farZ = baseEnvCameraComponent::DEFAULT_FAR_Z;
		math::Degree fov = baseEnvCameraComponent::DEFAULT_FOV;

		std::function<bool(ecs::BaseEntity &)> entityFilter = nullptr;
		const std::vector<ecs::BaseEntity *> *entityList = nullptr;
	};
	util::ParallelJob<image::ImageLayerSet> render_image(ClientState &client, const SceneInfo &sceneInfo, const RenderImageInfo &renderImageInfo);
	util::ParallelJob<image::ImageLayerSet> bake_ambient_occlusion(ClientState &client, const SceneInfo &sceneInfo, asset::Model &mdl, uint32_t materialIndex);
	util::ParallelJob<image::ImageLayerSet> bake_ambient_occlusion(ClientState &client, const SceneInfo &sceneInfo, ecs::BaseEntity &ent, uint32_t materialIndex);
	util::ParallelJob<image::ImageLayerSet> bake_lightmaps(ClientState &client, const SceneInfo &sceneInfo);
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::rendering::cycles::SceneInfo::SceneFlags)
};

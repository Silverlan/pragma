// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:rendering.cycles;

export import pragma.shared;

export class ClientState;
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
		SceneFlags sceneFlags = static_cast<SceneFlags>(umath::to_integral(SceneFlags::CullObjectsOutsidePvs) | umath::to_integral(SceneFlags::CullObjectsOutsideCameraFrustum));
		DeviceType device = DeviceType::CPU;
		std::string sky = "";
		float skyStrength = 1.f;
		EulerAngles skyAngles = {};
		uint32_t maxTransparencyBounces = 64;
		float exposure = 1.f;
		std::optional<ColorTransform> colorTransform {};
	};
	struct DLLCLIENT RenderImageInfo {
		umath::Transform camPose {};
		bool equirectPanorama = false;
		Mat4 viewProjectionMatrix = {};
		float nearZ = pragma::baseEnvCameraComponent::DEFAULT_NEAR_Z;
		float farZ = pragma::baseEnvCameraComponent::DEFAULT_FAR_Z;
		umath::Degree fov = baseEnvCameraComponent::DEFAULT_FOV;

		std::function<bool(pragma::ecs::BaseEntity &)> entityFilter = nullptr;
		const std::vector<pragma::ecs::BaseEntity *> *entityList = nullptr;
	};
	util::ParallelJob<uimg::ImageLayerSet> render_image(ClientState &client, const SceneInfo &sceneInfo, const RenderImageInfo &renderImageInfo);
	util::ParallelJob<uimg::ImageLayerSet> bake_ambient_occlusion(ClientState &client, const SceneInfo &sceneInfo, pragma::Model &mdl, uint32_t materialIndex);
	util::ParallelJob<uimg::ImageLayerSet> bake_ambient_occlusion(ClientState &client, const SceneInfo &sceneInfo, pragma::ecs::BaseEntity &ent, uint32_t materialIndex);
	util::ParallelJob<uimg::ImageLayerSet> bake_lightmaps(ClientState &client, const SceneInfo &sceneInfo);
	using namespace umath::scoped_enum::bitwise;
};
export {
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<pragma::rendering::cycles::SceneInfo::SceneFlags> : std::true_type {};
	}
};

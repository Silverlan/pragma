// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:util.baking;

import :entities.components.lights.light;
export import :rendering.lightmap_data_cache;

export {
	namespace pragma::util::baking {
		struct DLLCLIENT LightSource {
			enum class Type : uint8_t { Point = 0, Spot, Directional };
			Vector3 position;
			Vector3 direction;
			math::Degree innerConeAngle;
			math::Degree outerConeAngle;
			Candela intensity;
			Vector3 color;
			Type type;
		};
		DLLCLIENT ParallelJob<std::shared_ptr<image::ImageBuffer>> bake_directional_lightmap_atlas(const std::vector<CLightComponent *> &lights, const std::vector<geometry::ModelSubMesh *> meshes, const std::vector<ecs::BaseEntity *> &entities, uint32_t width, uint32_t height,
		  rendering::LightmapDataCache *optLightmapDataCache = nullptr);
	};
};

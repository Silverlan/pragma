// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:util.baking;

import :entities.components.lights.light;
export import :rendering.lightmap_data_cache;

export {
	namespace util::baking {
		struct DLLCLIENT LightSource {
			enum class Type : uint8_t { Point = 0, Spot, Directional };
			Vector3 position;
			Vector3 direction;
			umath::Degree innerConeAngle;
			umath::Degree outerConeAngle;
			Candela intensity;
			Vector3 color;
			Type type;
		};
		DLLCLIENT util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> bake_directional_lightmap_atlas(const std::vector<::pragma::CLightComponent *> &lights, const std::vector<pragma::ModelSubMesh *> meshes, const std::vector<pragma::ecs::BaseEntity *> &entities, uint32_t width,
		  uint32_t height, ::pragma::LightmapDataCache *optLightmapDataCache = nullptr);
	};
};

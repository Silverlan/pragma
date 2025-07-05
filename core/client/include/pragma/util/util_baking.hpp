// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_UTIL_BAKING_HPP__
#define __PRAGMA_UTIL_BAKING_HPP__

#include "pragma/clientdefinitions.h"
#include <sharedutils/util_parallel_job.hpp>
#include <mathutil/umath_lighting.hpp>
#include <vector>

namespace pragma {
	class CLightComponent;
	struct LightmapDataCache;
};
namespace uimg {
	class ImageBuffer;
};
class ModelSubMesh;
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
	DLLCLIENT util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> bake_directional_lightmap_atlas(const std::vector<::pragma::CLightComponent *> &lights, const std::vector<ModelSubMesh *> meshes, const std::vector<BaseEntity *> &entities, uint32_t width, uint32_t height,
	  ::pragma::LightmapDataCache *optLightmapDataCache = nullptr);
};

#endif

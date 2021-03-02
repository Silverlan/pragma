/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GAME_LIMITS_H__
#define __GAME_LIMITS_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class GameLimits : uint32_t
{
	// Note: These have to match shaders/modules/sh_limits.gls!
	MaxUniformBufferSize = 16'384,
	MaxAbsoluteLights = 192, // MaxUniformBufferSize /sizeof(LightBufferData) (16k = common max size for uniform buffer)
	// MaxAbsoluteLights = 1'024, // TODO: Use this limit for storage buffers (test performance with uniform)
	MaxAbsoluteShadowLights = 20,
	MaxCSMCascades = 4,
	MaxDirectionalLightSources = 4,

	MaxActiveShadowMaps = 5, // Spot lights
	MaxActiveShadowCubeMaps = 5, // Point lights

	MaxMeshVertices = 1'872'457,
	MaxWorldDistance = 1'048'576, // Maximum reasonable distance; Used for raycasts, among other things
	MaxRayCastRange = 65'536,

	MaxBones = 1'024, // Maximum number of bones per entity; Has to be the same as the value used in shaders
	MaxVertexWeights = 8, // Maximum number of bone weights per vertex. Has to be lower than 8, otherwise additional changes are required. Also has to match the value used in the shaders.
	MaxImageArrayLayers = 2'048, // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxImageArrayLayers

	MaxEntityInstanceCount = 1'310'720 // Maximum number instanced entities that can be visible in one frame
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(GameLimits);

#endif

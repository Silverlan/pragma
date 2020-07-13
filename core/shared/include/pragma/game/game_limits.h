/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __GAME_LIMITS_H__
#define __GAME_LIMITS_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class GameLimits : uint32_t
{
	MaxAbsoluteLights = 1'024,
	MaxAbsoluteShadowLights = 20,
	MaxCSMCascades = 4,
	MaxDirectionalLightSources = 4,

	MaxActiveShadowMaps = 5, // Spot lights
	MaxActiveShadowCubeMaps = 5, // Point lights

	MaxMeshVertices = 1'872'457,
	MaxWorldDistance = 1'048'576, // Maximum reasonable distance; Used for raycasts, among other things
	MaxRayCastRange = 65'536,

	MaxBones = 512, // Maximum number of bones per entity; Has to be the same as the value used in shaders
	MaxVertexWeights = 8, // Maximum number of bone weights per vertex. Has to be lower than 8, otherwise additional changes are required. Also has to match the value used in the shaders.
	MaxImageArrayLayers = 2'048 // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxImageArrayLayers
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(GameLimits);

#endif

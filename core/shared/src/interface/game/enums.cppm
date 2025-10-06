// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cinttypes>
#include "mathutil/transform.hpp"
#include <mathutil/umath.h>

export module pragma.shared:game.enums;

export {
    // These have to match GLFW_RELEASE and GLFW_PRESS
	uint32_t KEY_RELEASE = 0;
	uint32_t KEY_PRESS = 1;

    constexpr uint32_t RESOURCE_TRANSFER_FRAGMENT_SIZE = 2'048;

    enum class TimerType : uint8_t { CurTime = 0, RealTime, ServerTime };

	enum DAMAGETYPE : uint32_t { GENERIC = 0, EXPLOSION = 1, BULLET = 2, BASH = 32, CRUSH = 64, SLASH = 128, ELECTRICITY = 256, ENERGY = 512, FIRE = 1024, PLASMA = 2048, IGNITE = FIRE | 4096 };

    enum class GameLimits : uint32_t {
        // Note: These have to match shaders/modules/sh_limits.gls!
        MaxUniformBufferSize = 16'384,
        MaxAbsoluteLights = 192, // MaxUniformBufferSize /sizeof(LightBufferData) (16k = common max size for uniform buffer)
        // MaxAbsoluteLights = 1'024, // TODO: Use this limit for storage buffers (test performance with uniform)
        MaxAbsoluteShadowLights = 20,
        MaxCSMCascades = 4,
        MaxDirectionalLightSources = 4,

        MaxActiveShadowMaps = 5,     // Spot lights
        MaxActiveShadowCubeMaps = 5, // Point lights

        MaxMeshVertices = 1'872'457,
        MaxWorldDistance = 1'048'576, // Maximum reasonable distance; Used for raycasts, among other things
        MaxRayCastRange = 65'536,

        MaxBones = 1'024,            // Maximum number of bones per entity; Has to be the same as the value used in shaders
        MaxVertexWeights = 8,        // Maximum number of bone weights per vertex. Has to be lower than 8, otherwise additional changes are required. Also has to match the value used in the shaders.
        MaxImageArrayLayers = 2'048, // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxImageArrayLayers

        MaxEntityInstanceCount = 1'310'720 // Maximum number instanced entities that can be visible in one frame
    };
    REGISTER_BASIC_ARITHMETIC_OPERATORS(GameLimits);

    namespace pragma {
        enum class CoordinateSpace : uint8_t {
            World = umath::to_integral(umath::CoordinateSpace::World),
            Local = umath::to_integral(umath::CoordinateSpace::Local),
            Object = umath::to_integral(umath::CoordinateSpace::Object),
        };
    };

    enum class LuaEntityType : uint8_t { NetworkLocal = 0u, Shared, Default = NetworkLocal };
};

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "util_enum_flags.hpp"

export module pragma.shared:game.enums;

export import pragma.math;

export {
	// These have to match GLFW_RELEASE and GLFW_PRESS
	CONSTEXPR_COMPAT uint32_t KEY_RELEASE = 0;
	CONSTEXPR_COMPAT uint32_t KEY_PRESS = 1;

	constexpr uint32_t RESOURCE_TRANSFER_FRAGMENT_SIZE = 2'048;

	enum class TimerType : uint8_t { CurTime = 0, RealTime, ServerTime };

	enum DamageType : uint32_t { Generic = 0, Explosion = 1, Bullet = 2, Bash = 32, Crush = 64, Slash = 128, Electricity = 256, Energy = 512, Fire = 1024, Plasma = 2048, Ignite = Fire | 4096 };

	namespace pragma {
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
		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::GameLimits)

	namespace pragma {
		enum class CoordinateSpace : uint8_t {
			World = pragma::math::to_integral(math::CoordinateSpace::World),
			Local = pragma::math::to_integral(math::CoordinateSpace::Local),
			Object = pragma::math::to_integral(math::CoordinateSpace::Object),
		};
	};

	enum class LuaEntityType : uint8_t { NetworkLocal = 0u, Shared, Default = NetworkLocal };
};

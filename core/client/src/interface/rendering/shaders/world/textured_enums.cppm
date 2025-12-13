// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.shaders.textured_enums;

export import pragma.math;

export namespace pragma {
	const float DefaultParallaxHeightScale = 0.025f;
	const uint16_t DefaultParallaxSteps = 16;
	const float DefaultAlphaDiscardThreshold = 0.99f;

	enum class GameShaderSpecializationConstantFlag : uint32_t {
		None = 0u,

		// Static
		EnableLightMapsBit = 1u,
		EnableAnimationBit = EnableLightMapsBit << 1u,
		EnableMorphTargetAnimationBit = EnableAnimationBit << 1u,

		EnableTranslucencyBit = EnableMorphTargetAnimationBit << 1u,

		PermutationCount = (EnableTranslucencyBit << 1u) - 1,
		Last = EnableTranslucencyBit
	};

	enum class GameShaderSpecializationPropertyIndex : uint32_t {
		Start = math::get_least_significant_set_bit_index_c(math::to_integral(GameShaderSpecializationConstantFlag::Last)) + 1,
		ShadowQuality = Start,
		DebugModeEnabled,
		BloomOutputEnabled,
		EnableSsao,
		EnableIbl,
		EnableDynamicLighting,
		EnableDynamicShadows
	};
	enum class GameShaderSpecialization : uint32_t { Generic = 0, Lightmapped, Animated, Count };
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::GameShaderSpecializationConstantFlag)
};

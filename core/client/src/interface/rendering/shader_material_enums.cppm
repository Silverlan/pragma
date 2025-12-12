// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.shader_material_enums;

export import pragma.math;

export namespace pragma::rendering::shader_material {
	constexpr size_t MAX_MATERIAL_SIZE = 128; // Max size per material in bytes

	enum class MaterialFlags : uint32_t {
		None = 0,
		HasNormalMap = 1,
		HasParallaxMap = HasNormalMap << 1,
		HasEmissionMap = HasParallaxMap << 1,
		Translucent = HasEmissionMap << 1,
		BlackToAlpha = Translucent << 1,

		GlowMode1 = BlackToAlpha << 1,
		GlowMode2 = GlowMode1 << 1,
		GlowMode3 = GlowMode2 << 1,
		GlowMode4 = GlowMode3 << 1,

		// See MAX_NUMBER_OF_SRGB_TEXTURES
		Srgb0 = GlowMode4 << 1u,
		Srgb1 = Srgb0 << 1u,
		Srgb2 = Srgb1 << 1u,
		Srgb3 = Srgb2 << 1u,

		Debug = Srgb3 << 1u,

		HasWrinkleMaps = Debug << 1u,
		HasRmaMap = HasWrinkleMaps << 1u,

		User = HasRmaMap << 1u,

		Last = User,
	};
	constexpr uint32_t MAX_NUMBER_OF_SRGB_TEXTURES = 4;
	using namespace pragma::math::scoped_enum::bitwise;
};
export {REGISTER_ENUM_FLAGS(pragma::rendering::shader_material::MaterialFlags)}

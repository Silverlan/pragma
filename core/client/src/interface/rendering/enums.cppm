// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.enums;

export import pragma.math;

export namespace pragma::rendering {
	enum class ShadowType : uint8_t { None = 0, StaticOnly = 1, Full = StaticOnly | 2 };

	DLLCLIENT bool VERBOSE_RENDER_OUTPUT_ENABLED = false;

	enum class ShadowMapType : uint8_t { Static = 0, Dynamic };
	using namespace pragma::math::scoped_enum::bitwise;

	enum class RenderFlags : uint32_t {
		None = 0,
		World = 1,
		View = World << 1,
		Skybox = View << 1,
		Shadows = Skybox << 1,
		Particles = Shadows << 1,
		Debug = Particles << 1,
		Water = Debug << 1,
		Static = Water << 1,
		Dynamic = Static << 1,
		Translucent = Dynamic << 1,

		All = (Translucent << 1) - 1,
		Reflection = Translucent << 1,
		HDR = Reflection << 1,
		ParticleDepth = HDR << 1
	};

	enum class SceneRenderPass : uint8_t { None = 0, World, View, Sky, Count };

	enum class RenderMask : uint64_t { None = 0u };
	using RenderGroup = RenderMask;

	enum class AntiAliasing : uint8_t { None = 0u, MSAA, FXAA };
	enum class ToneMapping : uint32_t { None = 0u, GammaCorrection, Reinhard, HejilRichard, Uncharted, Aces, GranTurismo };

	DLLCLIENT int GetMaxMSAASampleCount();
	DLLCLIENT unsigned char ClampMSAASampleCount(unsigned int *samples);
};

export {
	REGISTER_ENUM_FLAGS(pragma::rendering::ShadowType)
	REGISTER_ENUM_FLAGS(pragma::rendering::RenderFlags)
	REGISTER_ENUM_FLAGS(pragma::rendering::RenderMask)
}

export namespace pragma::rendering {
	enum class ParticleAlphaMode : uint32_t {
		Additive = 0u,
		AdditiveByColor,
		Opaque,
		Masked,
		Translucent,
		Premultiplied,
		Custom,

		Count
	};
	DLLCLIENT bool premultiply_alpha(Vector4 &color, ParticleAlphaMode alphaMode);
};

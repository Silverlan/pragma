// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:debug.enums;

export import std.compat;

export namespace pragma {
	enum class SceneDebugMode : uint32_t {
		None = 0,
		AmbientOcclusion,
		Albedo,
		Metalness,
		Roughness,
		DiffuseLighting,
		Normal,
		NormalMap,
		Reflectance,
		IBLPrefilter,
		IBLIrradiance,
		Emission,
		Lightmap,
		LightmapUv,
		Unlit,
		CsmShowCascades,
		ShadowMapDepth,
		ForwardPlusHeatmap,
		Specular,
		IndirectLightmap,
		DirectionalLightmap,

		Count
	};
};

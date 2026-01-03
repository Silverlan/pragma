// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.enums;

export import std.compat;

export namespace pragma::pts {
	enum class ParticleRenderFlags : uint32_t { None = 0u, Bloom = 1u, DepthOnly = Bloom << 1u };
	enum class ParticleOrientationType : uint8_t {
		Aligned = 0,
		Upright,
		Static,
		World,
		Billboard,

		Velocity // Velocity is a special enum that shouldn't be set directly
	};
};

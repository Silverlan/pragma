// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.light_data;

export import pragma.math;
export import std.compat;

export namespace pragma {
#pragma pack(push, 1)
	struct LightBufferData {
		enum class BufferFlags : uint32_t { None = 0, TurnedOn = 1, TypeSpot = TurnedOn << 1, TypePoint = TypeSpot << 1, TypeDirectional = TypePoint << 1, BakedLightSource = TypeDirectional << 1 };
		Vector4 position {}; // position.w = distance
		Vector3 color {1.f, 1.f, 1.f};
		Candela intensity = 0.f;
		Vector4 direction {}; // direction.w is unused

		uint32_t sceneFlags = 0u;
		uint32_t shadowIndex = 0u;

		math::Radian outerConeHalfAngle = 0.f;
		math::Radian innerConeHalfAngle = 0.f;
		float attenuation = 0.f;
		BufferFlags flags = BufferFlags::None;
		uint32_t shadowMapIndexStatic = 0u;
		uint32_t shadowMapIndexDynamic = 0u;

		float falloffExponent = 1.f;
		std::array<float, 3> padding = {}; // Padding to vec4
	};
	struct ShadowBufferData {
		Mat4 depthVP = umat::identity();
		Mat4 view = umat::identity();
		Mat4 projection = umat::identity();
	};
#pragma pack(pop)
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::LightBufferData::BufferFlags)
};

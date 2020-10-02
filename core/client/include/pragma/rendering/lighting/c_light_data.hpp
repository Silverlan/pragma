/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LIGHT_DATA_MANAGER_HPP__
#define __C_LIGHT_DATA_MANAGER_HPP__

#include "pragma/clientdefinitions.h"
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <mathutil/uvec.h>
#include <mathutil/umath_lighting.hpp>
#include <cinttypes>
#include <array>

namespace pragma
{
#pragma pack(push,1)
	struct LightBufferData
	{
		enum class BufferFlags : uint32_t
		{
			None = 0,
			TurnedOn = 1,
			TypeSpot = TurnedOn<<1,
			TypePoint = TypeSpot<<1,
			TypeDirectional = TypePoint<<1,
			BakedLightSource = TypeDirectional<<1
		};
		Vector4 position {}; // position.w = distance
		Vector3 color {1.f,1.f,1.f};
		Candela intensity = 0.f;
		Vector4 direction {}; // direction.w is unused

		uint32_t sceneFlags = 0u;
		uint32_t shadowIndex = 0u;

		umath::Radian cutoffOuterCos = 0.f;
		umath::Radian cutoffInnerCos = 0.f;
		float attenuation = 0.f;
		BufferFlags flags = BufferFlags::None;
		uint32_t shadowMapIndexStatic = 0u;
		uint32_t shadowMapIndexDynamic = 0u;

		float falloffExponent = 1.f;
		std::array<float,3> padding = {}; // Padding to vec4
	};
	struct ShadowBufferData
	{
		Mat4 depthVP = umat::identity();
		Mat4 view = umat::identity();
		Mat4 projection = umat::identity();
	};
#pragma pack(pop)
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::LightBufferData::BufferFlags);

#endif

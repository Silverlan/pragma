#ifndef __C_LIGHT_DATA_MANAGER_HPP__
#define __C_LIGHT_DATA_MANAGER_HPP__

#include "pragma/clientdefinitions.h"
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <mathutil/uvec.h>
#include <cinttypes>

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
		Vector4 color {};
		Vector4 direction {}; // direction.w is unused
		uint32_t shadowIndex = 0u;

		float cutoffOuter = 0.f;
		float cutoffInner = 0.f;
		float attenuation = 0.f;
		BufferFlags flags = BufferFlags::None;
		uint32_t shadowMapIndexStatic = 0u;
		uint32_t shadowMapIndexDynamic = 0u;

		float falloffExponent = 1.f;
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

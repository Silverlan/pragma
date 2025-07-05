// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_TEXTURED_UNIFORM_DATA_HPP__
#define __C_SHADER_TEXTURED_UNIFORM_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include <mathutil/uvec.h>

namespace pragma {
#pragma pack(push, 1)
	struct RenderSettings {
		Vector3 posCam;
		umath::Radian fov;
		int32_t flags;
		float shadowRatioX;
		float shadowRatioY;
		float nearZ;
		float farZ;
		int32_t shaderQuality;
	};
	struct CameraData {
		Mat4 V;
		Mat4 P;
		Mat4 VP;
	};
	struct FogData {
		Vector4 color;
		float start;
		float end;
		float density;
		uint32_t type;
		uint32_t flags;
	};
#pragma pack(pop)
};

#endif

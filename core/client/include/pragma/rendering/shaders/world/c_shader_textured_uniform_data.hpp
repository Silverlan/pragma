/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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

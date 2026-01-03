// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.textured_uniform_data;

export import pragma.math;

export namespace pragma {
#pragma pack(push, 1)
	struct RenderSettings {
		Vector3 posCam;
		math::Radian fov;
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

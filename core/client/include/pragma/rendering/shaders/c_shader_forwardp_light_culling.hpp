/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_FORWARDP_LIGHT_CULLING_HPP__
#define __C_SHADER_FORWARDP_LIGHT_CULLING_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderForwardPLightCulling
		: public prosper::ShaderCompute
	{
	public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;

		static uint32_t TILE_SIZE;

		enum class CameraBinding : uint32_t
		{
			Camera = 0u,
			RenderSettings
		};

		enum class LightBinding : uint32_t
		{
			LightBuffers = 0u,
			TileVisLightIndexBuffer,
			ShadowData,
			VisLightIndexBuffer,
			DepthMap
		};

#pragma pack(push,1)
		struct PushConstants
		{
			uint32_t numLights;
			uint32_t sceneFlag;
			uint32_t viewportResolution;
		};
#pragma pack(pop)

		ShaderForwardPLightCulling(prosper::IPrContext &context,const std::string &identifier);
		bool Compute(
			prosper::IDescriptorSet &descSetLights,prosper::IDescriptorSet &descSetCamera,uint32_t vpWidth,uint32_t vpHeight,uint32_t workGroupsX,uint32_t workGroupsY,uint32_t lightCount,
			uint32_t sceneIndex
		);
	protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
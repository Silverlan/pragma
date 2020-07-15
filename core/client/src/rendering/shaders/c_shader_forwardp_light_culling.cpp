/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

uint32_t ShaderForwardPLightCulling::TILE_SIZE = 16u;
decltype(ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS) ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS = {
	{
		prosper::DescriptorSetInfo::Binding { // Light Buffers
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // Visible light tile index buffer
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // Shadow Buffers
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // Visible light index buffer
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // Depth Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::ComputeBit
		}
	}
};
decltype(ShaderForwardPLightCulling::DESCRIPTOR_SET_CAMERA) ShaderForwardPLightCulling::DESCRIPTOR_SET_CAMERA = {
	{
		prosper::DescriptorSetInfo::Binding { // Camera
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // Render Settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::ComputeBit
		}
	}
};
ShaderForwardPLightCulling::ShaderForwardPLightCulling(prosper::IPrContext &context,const std::string &identifier)
	: prosper::ShaderCompute(context,identifier,"compute/cs_forwardp_light_culling")
{}

void ShaderForwardPLightCulling::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::ComputeBit);
	// Currently not supported on some GPUs?
	// AddSpecializationConstant(pipelineInfo,0u /* constant id */,sizeof(TILE_SIZE),&TILE_SIZE);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
}

bool ShaderForwardPLightCulling::Compute(
	prosper::IDescriptorSet &descSetLights,prosper::IDescriptorSet &descSetCamera,uint32_t vpWidth,uint32_t vpHeight,uint32_t workGroupsX,uint32_t workGroupsY,uint32_t lightCount,
	uint32_t sceneIndex
)
{
	return RecordPushConstants(PushConstants{
			lightCount,1u<<sceneIndex,vpWidth<<16 | static_cast<uint16_t>(vpHeight)
		}) &&
		RecordBindDescriptorSet(descSetLights,DESCRIPTOR_SET_LIGHTS.setIndex) &&
		RecordBindDescriptorSet(descSetCamera,DESCRIPTOR_SET_CAMERA.setIndex) &&
		RecordDispatch(workGroupsX,workGroupsY);
}

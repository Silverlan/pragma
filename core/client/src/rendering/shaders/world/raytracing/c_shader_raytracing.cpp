/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/raytracing/c_shader_raytracing.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/rendering/renderers/raytracing_renderer.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <wgui/types/wirect.h>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT) ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT = {
	{
		prosper::DescriptorSetInfo::Binding { // Image
			prosper::DescriptorType::StorageImage,
			prosper::ShaderStageFlags::ComputeBit
		}
	}
};
decltype(ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE) ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE = {
	{
		prosper::DescriptorSetInfo::Binding { // Textures
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::ComputeBit,
			umath::to_integral(GameLimits::MaxImageArrayLayers)
		},
		prosper::DescriptorSetInfo::Binding { // Materials
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // SubMeshInfos
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // EntityInstanceData
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // BoneMatrices
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // VertexBuffer
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // IndexBuffer
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // VertexWeightBuffer
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // AlphaBuffer
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
	}
};

decltype(ShaderRayTracing::DESCRIPTOR_SET_SCENE) ShaderRayTracing::DESCRIPTOR_SET_SCENE = {&ShaderForwardPLightCulling::DESCRIPTOR_SET_SCENE};
decltype(ShaderRayTracing::DESCRIPTOR_SET_LIGHTS) ShaderRayTracing::DESCRIPTOR_SET_LIGHTS = {
	{
		prosper::DescriptorSetInfo::Binding { // Light Buffers
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		}
	}
};
decltype(ShaderRayTracing::DESCRIPTOR_SET_IBL) ShaderRayTracing::DESCRIPTOR_SET_IBL = {
	{
		prosper::DescriptorSetInfo::Binding { // Irradiance Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // Prefilter Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // BRDF Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::ComputeBit
		}
	}
};
ShaderRayTracing::ShaderRayTracing(prosper::IPrContext &context,const std::string &identifier)
	: prosper::ShaderCompute(context,identifier,"world/raytracing/raytracing.gls")
{}

void ShaderRayTracing::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	// Currently not supported on some GPUs?
	// AddSpecializationConstant(pipelineInfo,0u /* constant id */,sizeof(TILE_SIZE),&TILE_SIZE);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::ComputeBit);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_IMAGE_OUTPUT);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_GAME_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_IBL);
}

bool ShaderRayTracing::Compute(
	const PushConstants &pushConstants,
	prosper::IDescriptorSet &descSetOutputImage,prosper::IDescriptorSet &descSetGameScene,
	prosper::IDescriptorSet &descSetCamera,prosper::IDescriptorSet &descSetLightSources,
	prosper::IDescriptorSet *descSetIBL,
	uint32_t workGroupsX,uint32_t workGroupsY
)
{
	return RecordBindDescriptorSets({
		&descSetOutputImage,
		&descSetGameScene,
		&descSetCamera,
		&descSetLightSources
	}) && (descSetIBL == nullptr || RecordBindDescriptorSet(*descSetIBL,DESCRIPTOR_SET_IBL.setIndex)) && RecordPushConstants(pushConstants) && RecordDispatch(workGroupsX,workGroupsY);
}

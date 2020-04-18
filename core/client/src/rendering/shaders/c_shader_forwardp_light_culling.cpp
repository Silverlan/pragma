#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

uint32_t ShaderForwardPLightCulling::TILE_SIZE = 16u;
decltype(ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS) ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Light Buffers
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Visible light tile index buffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Shadow Buffers
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Visible light index buffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Depth Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
decltype(ShaderForwardPLightCulling::DESCRIPTOR_SET_CAMERA) ShaderForwardPLightCulling::DESCRIPTOR_SET_CAMERA = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Camera
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Render Settings
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
ShaderForwardPLightCulling::ShaderForwardPLightCulling(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderCompute(context,identifier,"compute/cs_forwardp_light_culling")
{}

void ShaderForwardPLightCulling::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::COMPUTE_BIT);
	// Currently not supported on some GPUs?
	// AddSpecializationConstant(pipelineInfo,0u /* constant id */,sizeof(TILE_SIZE),&TILE_SIZE);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
}

bool ShaderForwardPLightCulling::Compute(
	Anvil::DescriptorSet &descSetLights,Anvil::DescriptorSet &descSetCamera,uint32_t workGroupsX,uint32_t workGroupsY,uint32_t lightCount,
	uint32_t sceneIndex
)
{
	return RecordPushConstants(PushConstants{
			lightCount,1u<<sceneIndex
		}) &&
		RecordBindDescriptorSet(descSetLights,DESCRIPTOR_SET_LIGHTS.setIndex) &&
		RecordBindDescriptorSet(descSetCamera,DESCRIPTOR_SET_CAMERA.setIndex) &&
		RecordDispatch(workGroupsX,workGroupsY);
}

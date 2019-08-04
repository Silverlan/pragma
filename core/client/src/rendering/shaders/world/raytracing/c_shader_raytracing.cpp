#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/raytracing/c_shader_raytracing.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/rendering/renderers/raytracing_renderer.hpp"
#include <misc/compute_pipeline_create_info.h>
#include <wgui/types/wirect.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

decltype(ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT) ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Image
			Anvil::DescriptorType::STORAGE_IMAGE,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
decltype(ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE) ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Textures
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT,
			umath::to_integral(GameLimits::MaxImageArrayLayers)
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Materials
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // SubMeshInfos
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // EntityInstanceData
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // BoneMatrices
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // VertexBuffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // IndexBuffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // VertexWeightBuffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // AlphaBuffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
	}
};

decltype(ShaderRayTracing::DESCRIPTOR_SET_CAMERA) ShaderRayTracing::DESCRIPTOR_SET_CAMERA = {&ShaderForwardPLightCulling::DESCRIPTOR_SET_CAMERA};
decltype(ShaderRayTracing::DESCRIPTOR_SET_LIGHTS) ShaderRayTracing::DESCRIPTOR_SET_LIGHTS = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Light Buffers
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
ShaderRayTracing::ShaderRayTracing(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderCompute(context,identifier,"world/raytracing/raytracing.gls")
{}

void ShaderRayTracing::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	// Currently not supported on some GPUs?
	// AddSpecializationConstant(pipelineInfo,0u /* constant id */,sizeof(TILE_SIZE),&TILE_SIZE);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::COMPUTE_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_IMAGE_OUTPUT);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_GAME_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
}

bool ShaderRayTracing::Compute(
	const PushConstants &pushConstants,
	Anvil::DescriptorSet &descSetOutputImage,Anvil::DescriptorSet &descSetGameScene,
	Anvil::DescriptorSet &descSetCamera,Anvil::DescriptorSet &descSetLightSources,
	uint32_t workGroupsX,uint32_t workGroupsY
)
{
	return RecordBindDescriptorSets({
		&descSetOutputImage,
		&descSetGameScene,
		&descSetCamera,
		&descSetLightSources
	}) && RecordPushConstants(pushConstants) && RecordDispatch(workGroupsX,workGroupsY);
}

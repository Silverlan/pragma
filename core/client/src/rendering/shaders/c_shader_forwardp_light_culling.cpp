#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

const uint32_t ShaderForwardPLightCulling::TILE_SIZE = 16u;
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

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
}

bool ShaderForwardPLightCulling::Compute(Anvil::DescriptorSet &descSetLights,Anvil::DescriptorSet &descSetCamera,uint32_t workGroupsX,uint32_t workGroupsY,uint32_t lightCount)
{
	return RecordPushConstants(PushConstants{
			lightCount
		}) &&
		RecordBindDescriptorSet(descSetLights,DESCRIPTOR_SET_LIGHTS.setIndex) &&
		RecordBindDescriptorSet(descSetCamera,DESCRIPTOR_SET_CAMERA.setIndex) &&
		RecordDispatch(workGroupsX,workGroupsY);
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"

using namespace Shader;

LINK_SHADER_TO_CLASS(ForwardPLightCulling,forwardp_light_culling);

ForwardPLightCulling::ForwardPLightCulling()
	: Base("forwardp_light_culling","compute/cs_forwardp_light_culling")
{}

void ForwardPLightCulling::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::COMPUTE_BIT,0,1
	});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT}, // Light Buffers
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT}, // Visible light tile index buffer
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT}, // Shadow Buffers
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT}, // Visible light index buffer,
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Depth Map
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT}, // Camera
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Render Settings
	}));
}

Vulkan::DescriptorSet ForwardPLightCulling::CreateLightDescriptorSet()
{
	static auto shader = ShaderSystem::get_shader("forwardp_light_culling");
	if(!shader.IsValid())
		return nullptr;
	auto *textured = static_cast<TexturedBase3D*>(shader.get());
	auto *pipeline = textured->GetPipeline();
	if(pipeline != nullptr)
	{
		auto &layout = const_cast<Vulkan::ShaderPipeline*>(pipeline)->GetDescriptorSetLayout(umath::to_integral(DescSet::LightBuffers));
		auto &pool = pipeline->GetPipeline()->GetContext().GetDescriptorPool(layout->GetDescriptorType());
		auto descSet = pool->CreateDescriptorSet(layout);
		return descSet;
	}
	return nullptr;
}

void ForwardPLightCulling::Compute(const Vulkan::CommandBuffer &computeCmd,const Vulkan::DescriptorSet &descSetLights,const Vulkan::DescriptorSet &descSetCamera,uint32_t workGroupsX,uint32_t workGroupsY,uint32_t lightCount)
{
	auto &context = *m_context.get();
	auto &pipeline = *GetPipeline();
	auto &layout = pipeline.GetPipelineLayout();
	
	computeCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::COMPUTE_BIT,1,&lightCount);
	computeCmd->BindDescriptorSet(umath::to_integral(DescSet::LightBuffers),layout,{descSetLights,descSetCamera});
	computeCmd->Dispatch(workGroupsX,workGroupsY);
}
#endif
#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_forwardp_light_indexing.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

decltype(ShaderForwardPLightIndexing::DESCRIPTOR_SET_VISIBLE_LIGHT) ShaderForwardPLightIndexing::DESCRIPTOR_SET_VISIBLE_LIGHT = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Visible light tile index buffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Visible light index buffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};

ShaderForwardPLightIndexing::ShaderForwardPLightIndexing(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderCompute(context,identifier,"compute/cs_forwardp_light_indexing")
{}

void ShaderForwardPLightIndexing::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::COMPUTE_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_VISIBLE_LIGHT);
}

bool ShaderForwardPLightIndexing::Compute(Anvil::DescriptorSet &descSetLights,uint32_t tileCount)
{
	return RecordPushConstants(PushConstants{
			tileCount
		}) &&
		RecordBindDescriptorSet(descSetLights,DESCRIPTOR_SET_VISIBLE_LIGHT.setIndex) &&
		RecordDispatch();
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader_forwardp_light_indexing.hpp"

using namespace Shader;

LINK_SHADER_TO_CLASS(ForwardPLightIndexing,forwardp_light_indexing);

ForwardPLightIndexing::ForwardPLightIndexing()
	: Base("forwardp_light_indexing","compute/cs_forwardp_light_indexing")
{}

void ForwardPLightIndexing::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::COMPUTE_BIT,0,1
	});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT}, // Visible light tile index buffer
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Visible light index buffer
	}));
}

Vulkan::DescriptorSet ForwardPLightIndexing::CreateLightDescriptorSet()
{
	static auto shader = ShaderSystem::get_shader("forwardp_light_indexing");
	if(!shader.IsValid())
		return nullptr;
	auto *textured = static_cast<TexturedBase3D*>(shader.get());
	auto *pipeline = textured->GetPipeline();
	if(pipeline != nullptr)
	{
		auto &layout = const_cast<Vulkan::ShaderPipeline*>(pipeline)->GetDescriptorSetLayout(umath::to_integral(DescSet::TileVisLightIndexBuffer));
		auto &pool = pipeline->GetPipeline()->GetContext().GetDescriptorPool(layout->GetDescriptorType());
		auto descSet = pool->CreateDescriptorSet(layout);
		return descSet;
	}
	return nullptr;
}

void ForwardPLightIndexing::Compute(const Vulkan::CommandBuffer &computeCmd,const Vulkan::DescriptorSet &descSetLights,uint32_t tileCount)
{
	auto &context = *m_context.get();
	auto &pipeline = *GetPipeline();
	auto &layout = pipeline.GetPipelineLayout();
	
	computeCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::COMPUTE_BIT,1,&tileCount);
	computeCmd->BindDescriptorSet(umath::to_integral(DescSet::TileVisLightIndexBuffer),layout,descSetLights);
	computeCmd->Dispatch();
}
#endif
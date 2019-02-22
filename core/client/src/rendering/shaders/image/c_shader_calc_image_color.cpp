#include "stdafx_client.h"
#include "pragma/rendering/shaders/image/c_shader_calc_image_color.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

decltype(ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE) ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Texture
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
decltype(ShaderCalcImageColor::DESCRIPTOR_SET_COLOR) ShaderCalcImageColor::DESCRIPTOR_SET_COLOR = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Average Color
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
ShaderCalcImageColor::ShaderCalcImageColor(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderCompute(context,identifier,"compute/cs_calc_image_color")
{}

void ShaderCalcImageColor::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::COMPUTE_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_COLOR);
}

bool ShaderCalcImageColor::Compute(Anvil::DescriptorSet &descSetTexture,Anvil::DescriptorSet &descSetColor,uint32_t sampleCount)
{
	return RecordPushConstants(PushConstants{static_cast<int32_t>(sampleCount)}) &&
		RecordBindDescriptorSet(descSetTexture,DESCRIPTOR_SET_TEXTURE.setIndex) &&
		RecordBindDescriptorSet(descSetColor,DESCRIPTOR_SET_COLOR.setIndex) &&
		RecordDispatch();
}

 // prosper TODO
#if 0
#include "c_shader_calc_image_color.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(CalcImageColor,calcimagecolor);

decltype(CalcImageColor::SHADER_DESC_SET_TEXTURE) CalcImageColor::SHADER_DESC_SET_TEXTURE = 0;
decltype(CalcImageColor::SHADER_DESC_SET_COLOR) CalcImageColor::SHADER_DESC_SET_COLOR = 1;
CalcImageColor::CalcImageColor()
	: Base("calcimagecolor","compute/cs_calc_image_color")
{}

void CalcImageColor::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::COMPUTE_BIT,1
	});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Texture
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::STORAGE_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Average Color
	}));
}

Vulkan::DescriptorSet CalcImageColor::CreateColorDescriptorSet()
{
	static auto shader = ShaderSystem::get_shader("calcimagecolor");
	if(!shader.IsValid())
		return nullptr;
	auto *textured = static_cast<TexturedBase3D*>(shader.get());
	auto *pipeline = textured->GetPipeline();
	if(pipeline != nullptr)
	{
		auto &layout = const_cast<Vulkan::ShaderPipeline*>(pipeline)->GetDescriptorSetLayout(CalcImageColor::SHADER_DESC_SET_COLOR);
		auto &pool = pipeline->GetPipeline()->GetContext().GetDescriptorPool(layout->GetDescriptorType());
		auto descSet = pool->CreateDescriptorSet(layout);
		return descSet;
	}
	return nullptr;
}

Vulkan::DescriptorSet CalcImageColor::CreateTextureDescriptorSet()
{
	static auto shader = ShaderSystem::get_shader("calcimagecolor");
	if(!shader.IsValid())
		return nullptr;
	auto *textured = static_cast<TexturedBase3D*>(shader.get());
	auto *pipeline = textured->GetPipeline();
	if(pipeline != nullptr)
	{
		auto &layout = const_cast<Vulkan::ShaderPipeline*>(pipeline)->GetDescriptorSetLayout(CalcImageColor::SHADER_DESC_SET_TEXTURE);
		auto &pool = pipeline->GetPipeline()->GetContext().GetDescriptorPool(layout->GetDescriptorType());
		auto descSet = pool->CreateDescriptorSet(layout);
		return descSet;
	}
	return nullptr;
}

void CalcImageColor::Compute(const Vulkan::DescriptorSet &descColor,const Vulkan::DescriptorSet &descTexture,uint32_t sampleCount)
{
	auto &context = *m_context.get();
	auto &computeCmd = context.GetComputeCmd();
	auto &pipeline = *GetPipeline();
	auto &layout = pipeline.GetPipelineLayout();
	
	computeCmd->BindDescriptorSet(SHADER_DESC_SET_TEXTURE,layout,descTexture);

	computeCmd->BindDescriptorSet(SHADER_DESC_SET_COLOR,layout,descColor,{0});
	computeCmd->PushConstants(pipeline.GetPipelineLayout(),Anvil::ShaderStageFlagBits::COMPUTE_BIT,static_cast<int32_t>(sampleCount));
	computeCmd->Dispatch();
}
#endif
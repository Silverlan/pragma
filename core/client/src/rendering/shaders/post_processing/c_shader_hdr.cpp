#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include <shader/prosper_shader_copy_image.hpp>

using namespace pragma;

ShaderHDR::ShaderHDR(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderBaseImageProcessing(context,identifier,"screen/fs_hdr")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderHDR::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);
}

bool ShaderHDR::Draw(Anvil::DescriptorSet &descSetTexture,float exposure)
{
	return RecordPushConstants(PushConstants{exposure}) &&
		prosper::ShaderBaseImageProcessing::Draw(descSetTexture);
}

 // prosper TODO
#if 0
#include "c_shader_hdr.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(HDR,hdr);

HDR::HDR()
	: Screen("hdr","screen/vs_screen_uv","screen/fs_hdr")
{}

void HDR::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Screen::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,1});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}
	}));
}

bool HDR::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,float exposure)
{
	auto r = Screen::BeginDraw(cmdBuffer);
	if(r == false)
		return false;
	auto &pipeline = *GetPipeline();
	cmdBuffer->PushConstants(pipeline.GetPipelineLayout(),Anvil::ShaderStageFlagBits::FRAGMENT_BIT,exposure);
	return r;
}
#endif
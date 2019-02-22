#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include <prosper_util.hpp>
#include <shader/prosper_shader_copy_image.hpp>

using namespace pragma;

decltype(ShaderPPHDR::DESCRIPTOR_SET_TEXTURE) ShaderPPHDR::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Texture
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Bloom
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Glow
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderPPHDR::RENDER_PASS_FORMAT) ShaderPPHDR::RENDER_PASS_FORMAT = Anvil::Format::R8G8B8A8_UNORM;
ShaderPPHDR::ShaderPPHDR(prosper::Context &context,const std::string &identifier)
	: ShaderPPBase(context,identifier,"screen/fs_pp_hdr")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderPPHDR::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPPHDR>({{
		{
			RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::DONT_CARE,
			Anvil::AttachmentStoreOp::STORE,Anvil::SampleCountFlagBits::_1_BIT,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL
		}
	}},outRenderPass,pipelineIdx);
}

void ShaderPPHDR::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddDefaultVertexAttributes(pipelineInfo);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);
}

bool ShaderPPHDR::Draw(Anvil::DescriptorSet &descSetTexture,float exposure,float bloomScale,float glowScale,bool toneMappingOnly)
{
	return RecordPushConstants(PushConstants{exposure,bloomScale,glowScale,static_cast<int32_t>(toneMappingOnly)}) &&
		ShaderPPBase::Draw(descSetTexture) == true;
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/uniformbinding.h"

extern DLLCLIENT CGame *c_game;

using namespace Shader;

LINK_SHADER_TO_CLASS(PPHDR,pp_hdr);

PPHDR::PPHDR()
	: Screen("pp_hdr","screen/vs_screen_uv","screen/fs_pp_hdr")
{}

void PPHDR::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	//Screen::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,3});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Texture
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}, // Bloom
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Glow
	}));
}

void PPHDR::Draw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descTexture,const Vulkan::DescriptorSet &descBloomGlow,float exposure,float bloomScale,float glowScale)
{
	auto &pipeline = *GetPipeline();
	auto &layout = pipeline.GetPipelineLayout();
	std::array<float,3> pushConstants = {exposure,bloomScale,glowScale};
	cmdBuffer->PushConstants(pipeline.GetPipelineLayout(),Anvil::ShaderStageFlagBits::FRAGMENT_BIT,pushConstants.size(),pushConstants.data());
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::Bloom),layout,descBloomGlow);
	Screen::Draw(cmdBuffer,descTexture);
}
#endif

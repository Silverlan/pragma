#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/c_settings.hpp"
#include "pragma/console/c_cvar.h"
#include <pragma/console/convars.h>
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

static auto cvToneMapping = GetClientConVar("cl_render_tone_mapping");
bool ShaderPPHDR::Draw(Anvil::DescriptorSet &descSetTexture,float exposure,float bloomScale,float glowScale)
{
	auto toneMapping = rendering::ToneMapping::Reinhard;
	auto toneMappingCvarVal = cvToneMapping->GetInt();
	switch(toneMappingCvarVal)
	{
	case -1:
		break;
	default:
		toneMapping = static_cast<rendering::ToneMapping>(toneMappingCvarVal);
		break;
	}
	return RecordPushConstants(PushConstants{exposure,bloomScale,glowScale,toneMapping}) &&
		ShaderPPBase::Draw(descSetTexture) == true;
}

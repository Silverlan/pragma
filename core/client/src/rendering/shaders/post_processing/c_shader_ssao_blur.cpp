#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_shader_copy_image.hpp>
#include <prosper_util.hpp>
#include <random>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

ShaderSSAOBlur::ShaderSSAOBlur(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderBaseImageProcessing(context,identifier,"screen/fs_ssao_blur")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderSSAOBlur::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderSSAO>({{{
		ShaderSSAO::RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::DONT_CARE,
		Anvil::AttachmentStoreOp::STORE,Anvil::SampleCountFlagBits::_1_BIT,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
	}}},outRenderPass,pipelineIdx);
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include <random>

using namespace Shader;

LINK_SHADER_TO_CLASS(SSAOBlur,ssao_blur);

SSAOBlur::SSAOBlur()
	: Screen("ssao_blur","screen/vs_screen_uv","screen/fs_ssao_blur")
{}

void SSAOBlur::InitializeRenderPasses()
{
	m_renderPasses.push_back({m_context->GenerateRenderPass(vk::Format::eR8Unorm)});
}

void SSAOBlur::InitializeAttachments(std::vector<vk::PipelineColorBlendAttachmentState> &attachments)
{
	Screen::InitializeAttachments(attachments);
	attachments.front().setColorWriteMask(Anvil::ColorComponentFlagBits::R_BIT);
}
#endif
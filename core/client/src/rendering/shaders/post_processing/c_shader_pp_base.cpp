#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <prosper_util.hpp>

using namespace pragma;

ShaderPPBase::ShaderPPBase(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: prosper::ShaderBaseImageProcessing(context,identifier,vsShader,fsShader)
{}
ShaderPPBase::ShaderPPBase(prosper::Context &context,const std::string &identifier,const std::string &fsShader)
	: prosper::ShaderBaseImageProcessing(context,identifier,fsShader)
{}

void ShaderPPBase::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPPBase>({{{
		ShaderScene::RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::DONT_CARE,
		Anvil::AttachmentStoreOp::STORE,Anvil::SampleCountFlagBits::_1_BIT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
	}}},outRenderPass,pipelineIdx);
}

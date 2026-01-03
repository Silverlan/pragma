// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.ssao_blur;

import :engine;

using namespace pragma;

ShaderSSAOBlur::ShaderSSAOBlur(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing(context, identifier, "programs/post_processing/ssao_blur") { SetBaseShader<prosper::ShaderCopyImage>(); }

void ShaderSSAOBlur::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderSSAO>({{{ShaderSSAO::RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::ShaderReadOnlyOptimal}}}, outRenderPass,
	  pipelineIdx);
}

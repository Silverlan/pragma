// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client;


import :rendering.shaders.base;

using namespace pragma;

ShaderPPBase::ShaderPPBase(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader) : prosper::ShaderBaseImageProcessing(context, identifier, vsShader, fsShader) {}
ShaderPPBase::ShaderPPBase(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader) : prosper::ShaderBaseImageProcessing(context, identifier, fsShader) {}

void ShaderPPBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPPBase>({{{ShaderScene::RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::ColorAttachmentOptimal}}},
	  outRenderPass, pipelineIdx);
}

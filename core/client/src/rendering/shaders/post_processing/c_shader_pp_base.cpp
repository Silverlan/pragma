/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>

using namespace pragma;

ShaderPPBase::ShaderPPBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: prosper::ShaderBaseImageProcessing(context,identifier,vsShader,fsShader)
{}
ShaderPPBase::ShaderPPBase(prosper::IPrContext &context,const std::string &identifier,const std::string &fsShader)
	: prosper::ShaderBaseImageProcessing(context,identifier,fsShader)
{}

void ShaderPPBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPPBase>({{{
		ShaderScene::RENDER_PASS_FORMAT,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::DontCare,
		prosper::AttachmentStoreOp::Store,prosper::SampleCountFlags::e1Bit,prosper::ImageLayout::ColorAttachmentOptimal
	}}},outRenderPass,pipelineIdx);
}

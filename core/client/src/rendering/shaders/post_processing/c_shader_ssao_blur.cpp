/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <prosper_util.hpp>
#include <random>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

ShaderSSAOBlur::ShaderSSAOBlur(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderBaseImageProcessing(context, identifier, "programs/post_processing/ssao_blur") { SetBaseShader<prosper::ShaderCopyImage>(); }

void ShaderSSAOBlur::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderSSAO>({{{ShaderSSAO::RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::ShaderReadOnlyOptimal}}}, outRenderPass,
	  pipelineIdx);
}

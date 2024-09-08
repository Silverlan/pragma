/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <shader/prosper_shader_t.hpp>

using namespace pragma;

ShaderHDR::ShaderHDR(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderBaseImageProcessing(context, identifier, "programs/post_processing/hdr") { SetBaseShader<prosper::ShaderCopyImage>(); }

void ShaderHDR::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { prosper::ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderHDR::InitializeShaderResources()
{
	prosper::ShaderBaseImageProcessing::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderHDR::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, float exposure) const { return RecordPushConstants(bindState, PushConstants {exposure}) && prosper::ShaderBaseImageProcessing::RecordDraw(bindState, descSetTexture); }

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/image/c_shader_flip_image.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_texture.hpp>
#include <random>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

ShaderFlipImage::ShaderFlipImage(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderBaseImageProcessing(context, identifier, "programs/util/flip") { SetBaseShader<prosper::ShaderCopyImage>(); }

ShaderFlipImage::~ShaderFlipImage() {}

void ShaderFlipImage::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderFlipImage::InitializeShaderResources()
{
	ShaderBaseImageProcessing::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderFlipImage::RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTexture, bool flipHorizontally, bool flipVertically) const
{
	prosper::ShaderBindState bindState {cmd};
	if(RecordBeginDraw(bindState) == false)
		return false;

	PushConstants pushConstants {};
	pushConstants.flipHorizontally = static_cast<uint32_t>(flipHorizontally);
	pushConstants.flipVertically = static_cast<uint32_t>(flipVertically);
	auto res = RecordDraw(bindState, descSetTexture, pushConstants);
	RecordEndDraw(bindState);
	return res;
}

bool ShaderFlipImage::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, const PushConstants &pushConstants) const { return RecordPushConstants(bindState, pushConstants) && prosper::ShaderBaseImageProcessing::RecordDraw(bindState, descSetTexture); }

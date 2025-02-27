/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/shaders/image/c_shader_merge_images.hpp"
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

decltype(ShaderMergeImages::DESCRIPTOR_SET_TEXTURE_2D) ShaderMergeImages::DESCRIPTOR_SET_TEXTURE_2D = {
  "TEXTURE2",
  {prosper::DescriptorSetInfo::Binding {"TEXTURE2", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderMergeImages::ShaderMergeImages(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderBaseImageProcessing(context, identifier, "programs/util/merge") { SetBaseShader<prosper::ShaderCopyImage>(); }

ShaderMergeImages::~ShaderMergeImages() {}

void ShaderMergeImages::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderMergeImages::InitializeShaderResources()
{
	ShaderBaseImageProcessing::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE_2D);
}

bool ShaderMergeImages::RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetTexture2) const
{
	prosper::ShaderBindState bindState {cmd};
	if(RecordBeginDraw(bindState) == false)
		return false;

	auto res = RecordBindDescriptorSet(bindState, descSetTexture2, DESCRIPTOR_SET_TEXTURE_2D.setIndex) && prosper::ShaderBaseImageProcessing::RecordDraw(bindState, descSetTexture);
	RecordEndDraw(bindState);
	return res;
}

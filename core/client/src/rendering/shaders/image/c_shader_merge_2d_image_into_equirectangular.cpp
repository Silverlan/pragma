/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/shaders/image/c_shader_merge_2d_image_into_equirectangular.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

decltype(ShaderMerge2dImageIntoEquirectangular::DESCRIPTOR_SET_TEXTURE_2D) ShaderMerge2dImageIntoEquirectangular::DESCRIPTOR_SET_TEXTURE_2D = {
  "TEXTURE_2D",
  {prosper::DescriptorSetInfo::Binding {"TEXTURE_2D", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderMerge2dImageIntoEquirectangular::ShaderMerge2dImageIntoEquirectangular(prosper::IPrContext &context, const std::string &identifier)
    : prosper::ShaderBaseImageProcessing {context, identifier, "programs/util/merge_2d_image_into_equirectangular", "programs/util/merge_2d_image_into_equirectangular"}
{
}

void ShaderMerge2dImageIntoEquirectangular::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderMerge2dImageIntoEquirectangular::InitializeShaderResources()
{
	ShaderBaseImageProcessing::InitializeShaderResources();
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE_2D);
}

bool ShaderMerge2dImageIntoEquirectangular::RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTextureEquirect, prosper::IDescriptorSet &descSetTexture2d, CubeFace cubeFace, umath::Degree range)
{
	PushConstants pushConstants {};
	pushConstants.xFactor = range / 360.f;
	pushConstants.cubeFace = cubeFace;

	auto vertBuffer = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	auto uvBuffer = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareUvBuffer();
	auto numVerts = prosper::CommonBufferCache::GetSquareVertexCount();

	auto success = false;
	prosper::ShaderBindState bindState {cmd};
	if(RecordBeginDraw(bindState) == true) {
		success = RecordPushConstants(bindState, pushConstants) && RecordBindDescriptorSet(bindState, descSetTextureEquirect) && RecordBindDescriptorSet(bindState, descSetTexture2d, DESCRIPTOR_SET_TEXTURE_2D.setIndex)
		  && RecordBindVertexBuffers(bindState, {vertBuffer.get(), uvBuffer.get()}) && prosper::ShaderGraphics::RecordDraw(bindState, numVerts);
		RecordEndDraw(bindState);
	}
	return success;
}

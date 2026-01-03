// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.merge_2d_image_to_equirectangular;

import :engine;

using namespace pragma;

decltype(ShaderMerge2dImageIntoEquirectangular::DESCRIPTOR_SET_TEXTURE_2D) ShaderMerge2dImageIntoEquirectangular::DESCRIPTOR_SET_TEXTURE_2D = {
  "TEXTURE_2D",
  {prosper::DescriptorSetInfo::Binding {"TEXTURE_2D", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderMerge2dImageIntoEquirectangular::ShaderMerge2dImageIntoEquirectangular(prosper::IPrContext &context, const std::string &identifier)
    : ShaderBaseImageProcessing {context, identifier, "programs/util/merge_2d_image_into_equirectangular", "programs/util/merge_2d_image_into_equirectangular"}
{
}

void ShaderMerge2dImageIntoEquirectangular::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderMerge2dImageIntoEquirectangular::InitializeShaderResources()
{
	ShaderBaseImageProcessing::InitializeShaderResources();
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE_2D);
}

bool ShaderMerge2dImageIntoEquirectangular::RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTextureEquirect, prosper::IDescriptorSet &descSetTexture2d, CubeFace cubeFace, math::Degree range)
{
	PushConstants pushConstants {};
	pushConstants.xFactor = range / 360.f;
	pushConstants.cubeFace = cubeFace;

	auto vertBuffer = get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	auto uvBuffer = get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareUvBuffer();
	auto numVerts = prosper::CommonBufferCache::GetSquareVertexCount();

	auto success = false;
	prosper::ShaderBindState bindState {cmd};
	if(RecordBeginDraw(bindState) == true) {
		success = RecordPushConstants(bindState, pushConstants) && RecordBindDescriptorSet(bindState, descSetTextureEquirect) && RecordBindDescriptorSet(bindState, descSetTexture2d, DESCRIPTOR_SET_TEXTURE_2D.setIndex)
		  && RecordBindVertexBuffers(bindState, {vertBuffer.get(), uvBuffer.get()}) && ShaderGraphics::RecordDraw(bindState, numVerts);
		RecordEndDraw(bindState);
	}
	return success;
}

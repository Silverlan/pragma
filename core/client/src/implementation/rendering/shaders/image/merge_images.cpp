// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.merge_images;

import :engine;

using namespace pragma;

decltype(ShaderMergeImages::DESCRIPTOR_SET_TEXTURE_2D) ShaderMergeImages::DESCRIPTOR_SET_TEXTURE_2D = {
  "TEXTURE2",
  {prosper::DescriptorSetInfo::Binding {"TEXTURE2", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderMergeImages::ShaderMergeImages(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing(context, identifier, "programs/util/merge") { SetBaseShader<prosper::ShaderCopyImage>(); }

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

	auto res = RecordBindDescriptorSet(bindState, descSetTexture2, DESCRIPTOR_SET_TEXTURE_2D.setIndex) && ShaderBaseImageProcessing::RecordDraw(bindState, descSetTexture);
	RecordEndDraw(bindState);
	return res;
}

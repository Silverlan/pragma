// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.calc_image_color;

using namespace pragma;

decltype(ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE) ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURE",
  {prosper::DescriptorSetInfo::Binding {"TEXTURE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::ComputeBit}},
};
decltype(ShaderCalcImageColor::DESCRIPTOR_SET_COLOR) ShaderCalcImageColor::DESCRIPTOR_SET_COLOR = {
  "AVERAGE_COLOR",
  {prosper::DescriptorSetInfo::Binding {"OUTPUT_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}},
};
ShaderCalcImageColor::ShaderCalcImageColor(prosper::IPrContext &context, const std::string &identifier) : ShaderCompute(context, identifier, "programs/compute/calc_image_color") {}

void ShaderCalcImageColor::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx); }

void ShaderCalcImageColor::InitializeShaderResources()
{
	ShaderCompute::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::ComputeBit);

	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_COLOR);
}

bool ShaderCalcImageColor::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetColor, uint32_t sampleCount) const
{
	return RecordPushConstants(bindState, PushConstants {static_cast<int32_t>(sampleCount)}) && RecordBindDescriptorSet(bindState, descSetTexture, DESCRIPTOR_SET_TEXTURE.setIndex) && RecordBindDescriptorSet(bindState, descSetColor, DESCRIPTOR_SET_COLOR.setIndex)
	  && RecordDispatch(bindState);
}

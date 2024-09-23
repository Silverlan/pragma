/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/image/c_shader_calc_image_color.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>

using namespace pragma;

decltype(ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE) ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURE",
  {prosper::DescriptorSetInfo::Binding {"TEXTURE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::ComputeBit}},
};
decltype(ShaderCalcImageColor::DESCRIPTOR_SET_COLOR) ShaderCalcImageColor::DESCRIPTOR_SET_COLOR = {
  "AVERAGE_COLOR",
  {prosper::DescriptorSetInfo::Binding {"OUTPUT_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}},
};
ShaderCalcImageColor::ShaderCalcImageColor(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderCompute(context, identifier, "programs/compute/calc_image_color") {}

void ShaderCalcImageColor::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx); }

void ShaderCalcImageColor::InitializeShaderResources()
{
	prosper::ShaderCompute::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::ComputeBit);

	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_COLOR);
}

bool ShaderCalcImageColor::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetColor, uint32_t sampleCount) const
{
	return RecordPushConstants(bindState, PushConstants {static_cast<int32_t>(sampleCount)}) && RecordBindDescriptorSet(bindState, descSetTexture, DESCRIPTOR_SET_TEXTURE.setIndex) && RecordBindDescriptorSet(bindState, descSetColor, DESCRIPTOR_SET_COLOR.setIndex)
	  && ShaderCompute::RecordDispatch(bindState);
}

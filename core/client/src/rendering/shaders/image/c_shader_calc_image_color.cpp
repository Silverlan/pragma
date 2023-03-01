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

decltype(ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE) ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE = {{prosper::DescriptorSetInfo::Binding {// Texture
  prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::ComputeBit}}};
decltype(ShaderCalcImageColor::DESCRIPTOR_SET_COLOR) ShaderCalcImageColor::DESCRIPTOR_SET_COLOR = {{prosper::DescriptorSetInfo::Binding {// Average Color
  prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}}};
ShaderCalcImageColor::ShaderCalcImageColor(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderCompute(context, identifier, "compute/cs_calc_image_color") {}

void ShaderCalcImageColor::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx);

	AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(PushConstants), prosper::ShaderStageFlags::ComputeBit);

	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_COLOR);
}

bool ShaderCalcImageColor::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetColor, uint32_t sampleCount) const
{
	return RecordPushConstants(bindState, PushConstants {static_cast<int32_t>(sampleCount)}) && RecordBindDescriptorSet(bindState, descSetTexture, DESCRIPTOR_SET_TEXTURE.setIndex) && RecordBindDescriptorSet(bindState, descSetColor, DESCRIPTOR_SET_COLOR.setIndex)
	  && ShaderCompute::RecordDispatch(bindState);
}

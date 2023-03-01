/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_forwardp_light_indexing.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderForwardPLightIndexing::DESCRIPTOR_SET_VISIBLE_LIGHT) ShaderForwardPLightIndexing::DESCRIPTOR_SET_VISIBLE_LIGHT = {{prosper::DescriptorSetInfo::Binding {// Visible light tile index buffer
                                                                                                                                    prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
  prosper::DescriptorSetInfo::Binding {// Visible light index buffer
    prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}}};

ShaderForwardPLightIndexing::ShaderForwardPLightIndexing(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderCompute(context, identifier, "compute/cs_forwardp_light_indexing") {}

void ShaderForwardPLightIndexing::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx);

	AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(PushConstants), prosper::ShaderStageFlags::ComputeBit);

	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_VISIBLE_LIGHT);
}

bool ShaderForwardPLightIndexing::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetLights, uint32_t tileCount) const
{
	return RecordPushConstants(bindState, PushConstants {tileCount}) && RecordBindDescriptorSet(bindState, descSetLights, DESCRIPTOR_SET_VISIBLE_LIGHT.setIndex) && RecordDispatch(bindState);
}

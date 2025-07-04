// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/shaders/c_shader_forwardp_light_indexing.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderForwardPLightIndexing::DESCRIPTOR_SET_VISIBLE_LIGHT) ShaderForwardPLightIndexing::DESCRIPTOR_SET_VISIBLE_LIGHT = {
  "VISIBLE_LIGHT",
  {prosper::DescriptorSetInfo::Binding {"VISIBLE_LIGHT_TILE_INDEX_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"VISIBLE_LIGHT_INDEX_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}},
};

ShaderForwardPLightIndexing::ShaderForwardPLightIndexing(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderCompute(context, identifier, "programs/compute/forwardp_light_indexing") {}

void ShaderForwardPLightIndexing::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx); }

void ShaderForwardPLightIndexing::InitializeShaderResources()
{
	prosper::ShaderCompute::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::ComputeBit);
	AddDescriptorSetGroup(DESCRIPTOR_SET_VISIBLE_LIGHT);
}

bool ShaderForwardPLightIndexing::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetLights, uint32_t tileCount) const
{
	return RecordPushConstants(bindState, PushConstants {tileCount}) && RecordBindDescriptorSet(bindState, descSetLights, DESCRIPTOR_SET_VISIBLE_LIGHT.setIndex) && RecordDispatch(bindState);
}

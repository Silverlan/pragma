// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.light_data_buffer_manager;
import :rendering.shaders.forwardp_light_culling;

using namespace pragma;

uint32_t ShaderForwardPLightCulling::TILE_SIZE = 16u;
// Note: We *have* to call this descriptor set "RENDERER" because that's the same used in the scene shader
// and they have to match, otherwise the GLSL parser will not be able to resolve the descriptor set
decltype(ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS) ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS = {
  "RENDERER",
  {prosper::DescriptorSetInfo::Binding {"LIGHT_BUFFERS", LIGHT_SOURCE_BUFFER_TYPE, prosper::ShaderStageFlags::ComputeBit}, prosper::DescriptorSetInfo::Binding {"VISIBLE_LIGHT_TILE_INDEX_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"SHADOW_BUFFERS", LIGHT_SOURCE_BUFFER_TYPE, prosper::ShaderStageFlags::ComputeBit}, prosper::DescriptorSetInfo::Binding {"VISIBLE_LIGHT_INDEX_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"DEPTH_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::ComputeBit}},
};
decltype(ShaderForwardPLightCulling::DESCRIPTOR_SET_SCENE) ShaderForwardPLightCulling::DESCRIPTOR_SET_SCENE = {
  "SCENE",
  {
    prosper::DescriptorSetInfo::Binding {"CAMERA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"RENDER_SETTINGS", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::ComputeBit},
  },
};
ShaderForwardPLightCulling::ShaderForwardPLightCulling(prosper::IPrContext &context, const std::string &identifier) : ShaderCompute(context, identifier, "programs/compute/forwardp_light_culling") {}

void ShaderForwardPLightCulling::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx);

	// Currently not supported on some GPUs?
	// AddSpecializationConstant(pipelineInfo,0u /* constant id */,sizeof(TILE_SIZE),&TILE_SIZE);
}
void ShaderForwardPLightCulling::InitializeShaderResources()
{
	ShaderCompute::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::ComputeBit);
	// Currently not supported on some GPUs?
	// AddSpecializationConstant(pipelineInfo,0u /* constant id */,sizeof(TILE_SIZE),&TILE_SIZE);

	AddDescriptorSetGroup(DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
}

bool ShaderForwardPLightCulling::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetLights, prosper::IDescriptorSet &descSetCamera, uint32_t vpWidth, uint32_t vpHeight, uint32_t workGroupsX, uint32_t workGroupsY, uint32_t lightCount,
  uint32_t sceneIndex) const
{
	return RecordPushConstants(bindState, PushConstants {lightCount, 1u << sceneIndex, vpWidth << 16 | static_cast<uint16_t>(vpHeight)}) && RecordBindDescriptorSet(bindState, descSetLights, DESCRIPTOR_SET_LIGHTS.setIndex)
	  && RecordBindDescriptorSet(bindState, descSetCamera, DESCRIPTOR_SET_SCENE.setIndex) && RecordDispatch(bindState, workGroupsX, workGroupsY);
}

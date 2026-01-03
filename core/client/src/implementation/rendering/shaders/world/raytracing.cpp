// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_raytracing;

import :engine;

using namespace pragma;

decltype(ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT) ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT = {
  "OUTPUTS",
  {prosper::DescriptorSetInfo::Binding {"IMAGE", prosper::DescriptorType::StorageImage, prosper::ShaderStageFlags::ComputeBit}},
};
decltype(ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE) ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE = {
  "SCENE",
  {
    prosper::DescriptorSetInfo::Binding {"TEXTURES", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::ComputeBit, math::to_integral(GameLimits::MaxImageArrayLayers)},
    prosper::DescriptorSetInfo::Binding {"MATERIALS", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"SUB_MESH_INFOS", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"ENTITY_INSTANCE_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"BONE_MATRICES", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"VERTEX_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"INDEX_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"VERTEX_WEIGHT_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"ALPHA_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
  },
};

decltype(ShaderRayTracing::DESCRIPTOR_SET_SCENE) ShaderRayTracing::DESCRIPTOR_SET_SCENE = {&ShaderForwardPLightCulling::DESCRIPTOR_SET_SCENE};
decltype(ShaderRayTracing::DESCRIPTOR_SET_LIGHTS) ShaderRayTracing::DESCRIPTOR_SET_LIGHTS = {
  "LIGHTS",
  {prosper::DescriptorSetInfo::Binding {"LIGHT_BUFFERS", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}},
};
decltype(ShaderRayTracing::DESCRIPTOR_SET_IBL) ShaderRayTracing::DESCRIPTOR_SET_IBL = {
  "IBL",
  {prosper::DescriptorSetInfo::Binding {"IRRADIANCE_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::ComputeBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"PREFILTER_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::ComputeBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"BRDF_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::ComputeBit}},
};
ShaderRayTracing::ShaderRayTracing(prosper::IPrContext &context, const std::string &identifier) : ShaderCompute(context, identifier, "world/raytracing/raytracing") {}

void ShaderRayTracing::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx);

	// Currently not supported on some GPUs?
	// AddSpecializationConstant(pipelineInfo,0u /* constant id */,sizeof(TILE_SIZE),&TILE_SIZE);
}

void ShaderRayTracing::InitializeShaderResources()
{
	ShaderCompute::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::ComputeBit);

	AddDescriptorSetGroup(DESCRIPTOR_SET_IMAGE_OUTPUT);
	AddDescriptorSetGroup(DESCRIPTOR_SET_GAME_SCENE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(DESCRIPTOR_SET_IBL);
}

bool ShaderRayTracing::RecordCompute(prosper::ShaderBindState &bindState, const PushConstants &pushConstants, prosper::IDescriptorSet &descSetOutputImage, prosper::IDescriptorSet &descSetGameScene, prosper::IDescriptorSet &descSetCamera, prosper::IDescriptorSet &descSetLightSources,
  prosper::IDescriptorSet *descSetIBL, uint32_t workGroupsX, uint32_t workGroupsY) const
{
	return RecordBindDescriptorSets(bindState, {&descSetOutputImage, &descSetGameScene, &descSetCamera, &descSetLightSources}) && (descSetIBL == nullptr || RecordBindDescriptorSet(bindState, *descSetIBL, DESCRIPTOR_SET_IBL.setIndex)) && RecordPushConstants(bindState, pushConstants)
	  && RecordDispatch(bindState, workGroupsX, workGroupsY);
}

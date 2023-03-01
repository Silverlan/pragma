/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/lighting/c_light_data_buffer_manager.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

uint32_t ShaderForwardPLightCulling::TILE_SIZE = 16u;
decltype(ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS) ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS = {{prosper::DescriptorSetInfo::Binding {// Light Buffers
                                                                                                                    LIGHT_SOURCE_BUFFER_TYPE, prosper::ShaderStageFlags::ComputeBit},
  prosper::DescriptorSetInfo::Binding {// Visible light tile index buffer
    prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
  prosper::DescriptorSetInfo::Binding {// Shadow Buffers
    LIGHT_SOURCE_BUFFER_TYPE, prosper::ShaderStageFlags::ComputeBit},
  prosper::DescriptorSetInfo::Binding {// Visible light index buffer
    prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
  prosper::DescriptorSetInfo::Binding {// Depth Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::ComputeBit}}};
decltype(ShaderForwardPLightCulling::DESCRIPTOR_SET_SCENE) ShaderForwardPLightCulling::DESCRIPTOR_SET_SCENE = {{prosper::DescriptorSetInfo::Binding {// Camera
                                                                                                                  prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::ComputeBit},
  prosper::DescriptorSetInfo::Binding {// Render Settings
    prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::ComputeBit}}};
ShaderForwardPLightCulling::ShaderForwardPLightCulling(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderCompute(context, identifier, "compute/cs_forwardp_light_culling") {}

void ShaderForwardPLightCulling::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx);

	AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(PushConstants), prosper::ShaderStageFlags::ComputeBit);
	// Currently not supported on some GPUs?
	// AddSpecializationConstant(pipelineInfo,0u /* constant id */,sizeof(TILE_SIZE),&TILE_SIZE);

	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_SCENE);
}

bool ShaderForwardPLightCulling::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetLights, prosper::IDescriptorSet &descSetCamera, uint32_t vpWidth, uint32_t vpHeight, uint32_t workGroupsX, uint32_t workGroupsY, uint32_t lightCount,
  uint32_t sceneIndex) const
{
	return RecordPushConstants(bindState, PushConstants {lightCount, 1u << sceneIndex, vpWidth << 16 | static_cast<uint16_t>(vpHeight)}) && RecordBindDescriptorSet(bindState, descSetLights, DESCRIPTOR_SET_LIGHTS.setIndex)
	  && RecordBindDescriptorSet(bindState, descSetCamera, DESCRIPTOR_SET_SCENE.setIndex) && RecordDispatch(bindState, workGroupsX, workGroupsY);
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

using namespace pragma;

decltype(ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT) ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT = {
  "WATER",
  {prosper::DescriptorSetInfo::Binding {"PARTICLE_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}, prosper::DescriptorSetInfo::Binding {"POSITIONS", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit},
    prosper::DescriptorSetInfo::Binding {"TEMPORARY_PARTICLE_HEIGHTS", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}},
};
decltype(ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO) ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO = {
  "SURFACE",
  {prosper::DescriptorSetInfo::Binding {"INFO", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::ComputeBit}},
};
ShaderWaterSurface::ShaderWaterSurface(prosper::IPrContext &context, const std::string &identifier, const std::string &csShader) : prosper::ShaderCompute(context, identifier, csShader) {}
ShaderWaterSurface::ShaderWaterSurface(prosper::IPrContext &context, const std::string &identifier) : ShaderWaterSurface(context, identifier, "programs/compute/water/water_surface") {}
void ShaderWaterSurface::InitializeShaderResources()
{
	prosper::ShaderCompute::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_WATER_EFFECT);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SURFACE_INFO);
}
void ShaderWaterSurface::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx); }
bool ShaderWaterSurface::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetSurfaceInfo, prosper::IDescriptorSet &descSetParticles, uint32_t width, uint32_t length) const
{
	return RecordBindDescriptorSets(bindState, {&descSetParticles, &descSetSurfaceInfo}, DESCRIPTOR_SET_WATER_EFFECT.setIndex) && RecordDispatch(bindState, umath::ceil(width / 8.f), umath::ceil(length / 8.f), 1); //width,length); // TODO
}

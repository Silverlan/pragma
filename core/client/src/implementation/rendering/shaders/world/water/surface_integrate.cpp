// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_water_surface_integrate;

using namespace pragma;

decltype(ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES) ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES = {
  "WATER",
  {prosper::DescriptorSetInfo::Binding {"PARTICLE_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}},
};
decltype(ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_SURFACE_INFO) ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_SURFACE_INFO = {&ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO};
ShaderWaterSurfaceIntegrate::ShaderWaterSurfaceIntegrate(prosper::IPrContext &context, const std::string &identifier) : ShaderWaterSurface(context, identifier, "programs/compute/water/water_surface_integrate") {}
void ShaderWaterSurfaceIntegrate::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx); }
void ShaderWaterSurfaceIntegrate::InitializeShaderResources()
{
	ShaderCompute::InitializeShaderResources();

	AddDescriptorSetGroup(DESCRIPTOR_SET_WATER_PARTICLES);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SURFACE_INFO);
}

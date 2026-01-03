// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_water_surface;

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
ShaderWaterSurface::ShaderWaterSurface(prosper::IPrContext &context, const std::string &identifier, const std::string &csShader) : ShaderCompute(context, identifier, csShader) {}
ShaderWaterSurface::ShaderWaterSurface(prosper::IPrContext &context, const std::string &identifier) : ShaderWaterSurface(context, identifier, "programs/compute/water/water_surface") {}
void ShaderWaterSurface::InitializeShaderResources()
{
	ShaderCompute::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_WATER_EFFECT);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SURFACE_INFO);
}
void ShaderWaterSurface::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx); }
bool ShaderWaterSurface::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetSurfaceInfo, prosper::IDescriptorSet &descSetParticles, uint32_t width, uint32_t length) const
{
	return RecordBindDescriptorSets(bindState, {&descSetParticles, &descSetSurfaceInfo}, DESCRIPTOR_SET_WATER_EFFECT.setIndex) && RecordDispatch(bindState, math::ceil(width / 8.f), math::ceil(length / 8.f), 1); //width,length); // TODO
}

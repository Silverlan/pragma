// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :rendering.shaders.world_water_surface_sum_edges;

using namespace pragma;

decltype(ShaderWaterSurfaceSumEdges::DESCRIPTOR_SET_WATER) ShaderWaterSurfaceSumEdges::DESCRIPTOR_SET_WATER = {
  "WATER",
  {prosper::DescriptorSetInfo::Binding {"PARTICLE_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}, prosper::DescriptorSetInfo::Binding {"EDGE_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}},
};
decltype(ShaderWaterSurfaceSumEdges::DESCRIPTOR_SET_SURFACE_INFO) ShaderWaterSurfaceSumEdges::DESCRIPTOR_SET_SURFACE_INFO = {&ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO};
ShaderWaterSurfaceSumEdges::ShaderWaterSurfaceSumEdges(prosper::IPrContext &context, const std::string &identifier, const std::string &csShader) : prosper::ShaderCompute(context, identifier, csShader) {}
ShaderWaterSurfaceSumEdges::ShaderWaterSurfaceSumEdges(prosper::IPrContext &context, const std::string &identifier) : ShaderWaterSurfaceSumEdges(context, identifier, "programs/compute/water/water_surface_sum_edges") {}
void ShaderWaterSurfaceSumEdges::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx); }
void ShaderWaterSurfaceSumEdges::InitializeShaderResources()
{
	prosper::ShaderCompute::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_WATER);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SURFACE_INFO);
}
bool ShaderWaterSurfaceSumEdges::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetSurfaceInfo, prosper::IDescriptorSet &descSetEdges, uint32_t width, uint32_t length) const
{
	return RecordBindDescriptorSets(bindState, {&descSetEdges, &descSetSurfaceInfo}, DESCRIPTOR_SET_WATER.setIndex) && RecordDispatch(bindState, umath::ceil(width / 8.f), umath::ceil(length / 8.f), 1);
}

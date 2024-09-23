/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_sum_edges.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

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

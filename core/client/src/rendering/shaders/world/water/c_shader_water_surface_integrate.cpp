/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_integrate.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

using namespace pragma;

decltype(ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES) ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES = {{prosper::DescriptorSetInfo::Binding {// Water particles
  prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}}};
decltype(ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_SURFACE_INFO) ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_SURFACE_INFO = {&ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO};
ShaderWaterSurfaceIntegrate::ShaderWaterSurfaceIntegrate(prosper::IPrContext &context, const std::string &identifier) : ShaderWaterSurface(context, identifier, "compute/water/cs_water_surface_integrate") {}
void ShaderWaterSurfaceIntegrate::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_WATER_PARTICLES);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_SURFACE_INFO);
}

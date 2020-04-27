/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_integrate.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

decltype(ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES) ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES = {
	{
		prosper::DescriptorSetInfo::Binding { // Water particles
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		}
	}
};
decltype(ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_SURFACE_INFO) ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_SURFACE_INFO = {&ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO};
ShaderWaterSurfaceIntegrate::ShaderWaterSurfaceIntegrate(prosper::Context &context,const std::string &identifier)
	: ShaderWaterSurface(context,identifier,"compute/water/cs_water_surface_integrate")
{}
void ShaderWaterSurfaceIntegrate::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_WATER_PARTICLES);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SURFACE_INFO);
}

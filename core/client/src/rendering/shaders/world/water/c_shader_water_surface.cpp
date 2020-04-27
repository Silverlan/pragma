/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

decltype(ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT) ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT = {
	{
		prosper::DescriptorSetInfo::Binding { // Water particles
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // Water Positions
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // Temporary Particle Heights
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		}
	}
};
decltype(ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO) ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO = {
	{
		prosper::DescriptorSetInfo::Binding { // Surface info
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::ComputeBit
		}
	}
};
ShaderWaterSurface::ShaderWaterSurface(prosper::Context &context,const std::string &identifier,const std::string &csShader)
	: prosper::ShaderCompute(context,identifier,csShader)
{}
ShaderWaterSurface::ShaderWaterSurface(prosper::Context &context,const std::string &identifier)
	: ShaderWaterSurface(context,identifier,"compute/water/cs_water_surface")
{}
void ShaderWaterSurface::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_WATER_EFFECT);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SURFACE_INFO);
}
bool ShaderWaterSurface::Compute(prosper::IDescriptorSet &descSetSurfaceInfo,prosper::IDescriptorSet &descSetParticles,uint32_t width,uint32_t length)
{
	return RecordBindDescriptorSets({&descSetParticles,&descSetSurfaceInfo},DESCRIPTOR_SET_WATER_EFFECT.setIndex) &&
		RecordDispatch(umath::ceil(width /8.f),umath::ceil(length /8.f),1);//width,length); // TODO
}

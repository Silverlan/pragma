/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_splash.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

decltype(ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT) ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT = {
	{
		prosper::DescriptorSetInfo::Binding { // Water particles
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		},
		prosper::DescriptorSetInfo::Binding { // Water Positions
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::ComputeBit
		}
	}
};
ShaderWaterSplash::ShaderWaterSplash(prosper::IPrContext &context,const std::string &identifier)
	: prosper::ShaderCompute(context,identifier,"compute/water/cs_water_splash")
{}

void ShaderWaterSplash::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PhysWaterSurfaceSimulator::SplashInfo),prosper::ShaderStageFlags::ComputeBit);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_WATER_EFFECT);
}

bool ShaderWaterSplash::Compute(prosper::IDescriptorSet &descParticles,const PhysWaterSurfaceSimulator::SplashInfo &info)
{
	return RecordPushConstants(info) &&
		RecordBindDescriptorSet(descParticles,DESCRIPTOR_SET_WATER_EFFECT.setIndex) &&
		RecordDispatch(umath::ceil(info.width /8.f),umath::ceil(info.length /8.f),1);//width,length); // TODO
}

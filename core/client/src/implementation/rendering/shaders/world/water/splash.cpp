// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/umath.h"

#include "stdafx_client.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>

module pragma.client;


import :rendering.shaders.world_water_splash;

using namespace pragma;

decltype(ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT) ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT = {
  "WATER",
  {prosper::DescriptorSetInfo::Binding {"PARTICLE_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}, prosper::DescriptorSetInfo::Binding {"POSITIONS", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::ComputeBit}},
};
ShaderWaterSplash::ShaderWaterSplash(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderCompute(context, identifier, "programs/compute/water/water_splash") {}

void ShaderWaterSplash::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx); }

void ShaderWaterSplash::InitializeShaderResources()
{
	prosper::ShaderCompute::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PhysWaterSurfaceSimulator::SplashInfo), prosper::ShaderStageFlags::ComputeBit);
	AddDescriptorSetGroup(DESCRIPTOR_SET_WATER_EFFECT);
}

bool ShaderWaterSplash::RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descParticles, const PhysWaterSurfaceSimulator::SplashInfo &info) const
{
	return RecordPushConstants(bindState, info) && RecordBindDescriptorSet(bindState, descParticles, DESCRIPTOR_SET_WATER_EFFECT.setIndex) && RecordDispatch(bindState, umath::ceil(info.width / 8.f), umath::ceil(info.length / 8.f), 1); //width,length); // TODO
}

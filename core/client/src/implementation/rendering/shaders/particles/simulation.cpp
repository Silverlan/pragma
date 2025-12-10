// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle_simulation;

using namespace pragma;

pragma::rendering::shaders::ShaderParticleSimulation::ShaderParticleSimulation(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderCompute(context, identifier, "programs/compute/particles/simulation") {}

void pragma::rendering::shaders::ShaderParticleSimulation::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo, pipelineIdx); }

void pragma::rendering::shaders::ShaderParticleSimulation::InitializeShaderResources()
{
	prosper::ShaderCompute::InitializeShaderResources();
}

/*bool pragma::rendering::shaders::ShaderParticleSimulation::RecordCompute(prosper::ShaderBindState &bindState) const
{
	return ShaderCompute::RecordDispatch(bindState);
}*/

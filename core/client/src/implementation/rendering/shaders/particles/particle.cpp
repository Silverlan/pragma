// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle;

using namespace pragma;

ShaderParticle::ShaderParticle(prosper::IPrContext &context, const std::string &identifier) : ShaderParticle2DBase(context, identifier, "programs/particles/particle", "programs/particles/particle") {}

decltype(ShaderParticleRotational::VERTEX_BINDING_WORLD_ROTATION) ShaderParticleRotational::VERTEX_BINDING_WORLD_ROTATION = {prosper::VertexInputRate::Instance};
decltype(ShaderParticleRotational::VERTEX_ATTRIBUTE_WORLD_ROTATION) ShaderParticleRotational::VERTEX_ATTRIBUTE_WORLD_ROTATION = {VERTEX_BINDING_WORLD_ROTATION, prosper::Format::R32G32B32A32_SFloat};
ShaderParticleRotational::ShaderParticleRotational(prosper::IPrContext &context, const std::string &identifier) : ShaderParticle2DBase(context, identifier, "programs/particles/particle_rotational", "programs/particles/particle") { SetBaseShader<ShaderParticle>(); }

void ShaderParticleRotational::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderParticle2DBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo, prosper::CullModeFlags::None);
}

void ShaderParticleRotational::InitializeShaderResources()
{
	ShaderParticle2DBase::InitializeShaderResources();
	AddVertexAttribute(VERTEX_ATTRIBUTE_WORLD_ROTATION);
}

bool ShaderParticleRotational::RecordWorldRotationBuffer(prosper::ShaderBindState &bindState, prosper::IBuffer &buffer) const { return RecordBindVertexBuffer(bindState, buffer, VERTEX_BINDING_WORLD_ROTATION.GetBindingIndex()); }

void ShaderParticleRotational::GetParticleSystemOrientationInfo(const Mat4 &matrix, const ecs::CParticleSystemComponent &particle, Vector3 &up, Vector3 &right, float &nearZ, float &farZ, const material::Material *material, const BaseEnvCameraComponent *cam) const
{
	auto orType = particle.GetOrientationType();
	return ShaderParticle2DBase::GetParticleSystemOrientationInfo(matrix, particle, (orType == pts::ParticleOrientationType::Aligned) ? orType : pts::ParticleOrientationType::World, up, right, nearZ, farZ, material, cam);
}

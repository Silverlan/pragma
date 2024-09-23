/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>

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

void ShaderParticleRotational::GetParticleSystemOrientationInfo(const Mat4 &matrix, const pragma::CParticleSystemComponent &particle, Vector3 &up, Vector3 &right, float &nearZ, float &farZ, const Material *material, const pragma::CCameraComponent *cam) const
{
	auto orType = particle.GetOrientationType();
	return ShaderParticle2DBase::GetParticleSystemOrientationInfo(matrix, particle, (orType == pragma::CParticleSystemComponent::OrientationType::Aligned) ? orType : pragma::CParticleSystemComponent::OrientationType::World, up, right, nearZ, farZ, material, cam);
}

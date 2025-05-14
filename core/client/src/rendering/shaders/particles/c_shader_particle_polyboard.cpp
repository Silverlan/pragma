/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_polyboard.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <buffers/prosper_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

decltype(ShaderParticlePolyboard::VERTEX_BINDING_VERTEX) ShaderParticlePolyboard::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderParticlePolyboard::VERTEX_ATTRIBUTE_VERTEX) ShaderParticlePolyboard::VERTEX_ATTRIBUTE_VERTEX = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat};
decltype(ShaderParticlePolyboard::VERTEX_ATTRIBUTE_COLOR) ShaderParticlePolyboard::VERTEX_ATTRIBUTE_COLOR = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32A32_SFloat};
ShaderParticlePolyboard::ShaderParticlePolyboard(prosper::IPrContext &context, const std::string &identifier)
    : ShaderParticle2DBase(context, identifier, "programs/particles/beam/particle_polyboard", "programs/particles/beam/particle_polyboard", "programs/particles/beam/particle_polyboard")
{
	SetBaseShader<ShaderParticle>();
}

void ShaderParticlePolyboard::InitializeShaderResources()
{
	ShaderSceneLit::InitializeShaderResources();

	AddVertexAttribute(VERTEX_ATTRIBUTE_VERTEX);
	AddVertexAttribute(VERTEX_ATTRIBUTE_COLOR);

	AttachPushConstantRange(0u, sizeof(GeometryPushConstants), prosper::ShaderStageFlags::GeometryBit);
	AttachPushConstantRange(sizeof(GeometryPushConstants), sizeof(FragmentPushConstants), prosper::ShaderStageFlags::FragmentBit);

	RegisterDefaultGfxPipelineDescriptorSetGroups();
}

void ShaderParticlePolyboard::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	pipelineInfo.SetPrimitiveTopology(prosper::PrimitiveTopology::LineListWithAdjacency);
}

bool ShaderParticlePolyboard::Draw(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const pragma::CParticleSystemComponent &ps, prosper::IBuffer &vertexBuffer, prosper::IBuffer &indexBuffer, uint32_t numIndices, float radius, float curvature)
{
#if 0
	if(BindParticleMaterial(renderer,ps) == false)
		return false;
	auto &cam = scene.GetActiveCamera();
	if(RecordPushConstants(GeometryPushConstants{
		cam.valid() ? cam->GetEntity().GetPosition() : Vector3{},radius,curvature
			}) == false ||
		RecordPushConstants(FragmentPushConstants{
			static_cast<int32_t>(GetRenderFlags(ps,ParticleRenderFlags::None)), // TODO: Use correct particle render flags
			static_cast<float>(c_engine->GetRenderContext().GetWindowWidth()),
			static_cast<float>(c_engine->GetRenderContext().GetWindowHeight())
		},sizeof(GeometryPushConstants)) == false
		)
		return false;
	return RecordBindVertexBuffer(vertexBuffer) && RecordBindIndexBuffer(indexBuffer) && RecordDrawIndexed(numIndices);
#endif
	return false;
}

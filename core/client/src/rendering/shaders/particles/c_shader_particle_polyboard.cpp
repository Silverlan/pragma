/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_polyboard.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <buffers/prosper_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderParticlePolyboard::VERTEX_BINDING_VERTEX) ShaderParticlePolyboard::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderParticlePolyboard::VERTEX_ATTRIBUTE_VERTEX) ShaderParticlePolyboard::VERTEX_ATTRIBUTE_VERTEX = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32B32_SFloat};
decltype(ShaderParticlePolyboard::VERTEX_ATTRIBUTE_COLOR) ShaderParticlePolyboard::VERTEX_ATTRIBUTE_COLOR = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32B32A32_SFloat};
ShaderParticlePolyboard::ShaderParticlePolyboard(prosper::IPrContext &context,const std::string &identifier)
	: ShaderParticle2DBase(context,identifier,"particles/beam/vs_particle_polyboard","particles/beam/fs_particle_polyboard","particles/beam/gs_particle_polyboard")
{
	SetBaseShader<ShaderParticle>();
}

void ShaderParticlePolyboard::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_VERTEX);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_COLOR);

	pipelineInfo.SetPrimitiveTopology(prosper::PrimitiveTopology::LineListWithAdjacency);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(GeometryPushConstants),prosper::ShaderStageFlags::GeometryBit);
	AttachPushConstantRange(pipelineInfo,sizeof(GeometryPushConstants),sizeof(FragmentPushConstants),prosper::ShaderStageFlags::FragmentBit);

	RegisterDefaultGfxPipelineDescriptorSetGroups(pipelineInfo);
}

bool ShaderParticlePolyboard::Draw(const rendering::RasterizationRenderer &renderer,const pragma::CParticleSystemComponent &ps,prosper::IBuffer &vertexBuffer,prosper::IBuffer &indexBuffer,uint32_t numIndices,float radius,float curvature)
{
	if(BindParticleMaterial(renderer,ps) == false)
		return false;
	auto &cam = renderer.GetScene().GetActiveCamera();
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
}

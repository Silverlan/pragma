// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle_polyboard;

import :engine;

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

bool ShaderParticlePolyboard::Draw(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const ecs::CParticleSystemComponent &ps, prosper::IBuffer &vertexBuffer, prosper::IBuffer &indexBuffer, uint32_t numIndices, float radius, float curvature)
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
			static_cast<float>(pragma::get_cengine()->GetRenderContext().GetWindowWidth()),
			static_cast<float>(pragma::get_cengine()->GetRenderContext().GetWindowHeight())
		},sizeof(GeometryPushConstants)) == false
		)
		return false;
	return RecordBindVertexBuffer(vertexBuffer) && RecordBindIndexBuffer(indexBuffer) && RecordDrawIndexed(numIndices);
#endif
	return false;
}

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.pbr;

import :client_state;
import :engine;
import :entities.components;
import :model;

using namespace pragma;

decltype(ShaderPBRBlend::VERTEX_BINDING_ALPHA) ShaderPBRBlend::VERTEX_BINDING_ALPHA = {prosper::VertexInputRate::Vertex};
decltype(ShaderPBRBlend::VERTEX_ATTRIBUTE_ALPHA) ShaderPBRBlend::VERTEX_ATTRIBUTE_ALPHA = {VERTEX_BINDING_ALPHA, prosper::Format::R32G32_SFloat};
ShaderPBRBlend::ShaderPBRBlend(prosper::IPrContext &context, const std::string &identifier) : ShaderPBR {context, identifier, "programs/scene/textured_blend", "programs/scene/pbr/pbr_blend"} { m_shaderMaterialName = "pbr_blend"; }
void ShaderPBRBlend::InitializeGfxPipelineVertexAttributes()
{
	ShaderPBR::InitializeGfxPipelineVertexAttributes();
	AddVertexAttribute(VERTEX_ATTRIBUTE_ALPHA);
}
void ShaderPBRBlend::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }
bool ShaderPBRBlend::GetRenderBufferTargets(geometry::CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const
{
	if(ShaderPBR::GetRenderBufferTargets(mesh, pipelineIdx, outBuffers, outOffsets, outIndexBufferInfo) == false)
		return false;
	auto &sceneMesh = mesh.GetSceneMesh();
	auto *alphaBuf = sceneMesh->GetAlphaBuffer().get();
	outBuffers.push_back(alphaBuf);
	outOffsets.push_back(0ull);
	return true;
}
bool ShaderPBRBlend::OnRecordDrawMesh(rendering::ShaderProcessor &shaderProcessor, geometry::CModelSubMesh &mesh) const
{
	PushConstants pushConstants {};
	pushConstants.alphaCount = mesh.GetAlphaCount();
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, sizeof(ShaderGameWorldLightingPass::PushConstants), sizeof(pushConstants), &pushConstants);
	return ShaderPBR::OnRecordDrawMesh(shaderProcessor, mesh);
}

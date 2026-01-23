// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.textured_alpha_transition;

import :engine;
import :model;

using namespace pragma;

decltype(ShaderTexturedAlphaTransition::VERTEX_BINDING_ALPHA) ShaderTexturedAlphaTransition::VERTEX_BINDING_ALPHA = {prosper::VertexInputRate::Vertex};
decltype(ShaderTexturedAlphaTransition::VERTEX_ATTRIBUTE_ALPHA) ShaderTexturedAlphaTransition::VERTEX_ATTRIBUTE_ALPHA = {VERTEX_BINDING_ALPHA, prosper::Format::R32G32_SFloat};
ShaderTexturedAlphaTransition::ShaderTexturedAlphaTransition(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "world/vs_textured_alpha_transition", "world/fs_textured_alpha_transition")
{
	// SetBaseShader<ShaderTextured3DBase>();
}

void ShaderTexturedAlphaTransition::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }
void ShaderTexturedAlphaTransition::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
void ShaderTexturedAlphaTransition::InitializeShaderResources()
{
	ShaderGameWorldLightingPass::InitializeShaderResources();
	AddVertexAttribute(VERTEX_ATTRIBUTE_ALPHA);
}

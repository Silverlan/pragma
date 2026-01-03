// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.clear_color;

using namespace pragma;

decltype(ShaderClearColor::VERTEX_BINDING_VERTEX) ShaderClearColor::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderClearColor::VERTEX_ATTRIBUTE_POSITION) ShaderClearColor::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32_SFloat};
ShaderClearColor::ShaderClearColor(prosper::IPrContext &context, const std::string &identifier) : ShaderGraphics(context, identifier, "programs/image/noop", "programs/image/clear_color") {}

ShaderClearColor::ShaderClearColor(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderGraphics(context, identifier, vsShader, fsShader, gsShader) {}

void ShaderClearColor::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderClearColor::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderClearColor::RecordDraw(prosper::ShaderBindState &bindState, const PushConstants &pushConstants) const
{
	if(RecordBindVertexBuffer(bindState, *GetContext().GetCommonBufferCache().GetSquareVertexBuffer()) == false || RecordPushConstants(bindState, pushConstants) == false || ShaderGraphics::RecordDraw(bindState, prosper::CommonBufferCache::GetSquareVertexCount()) == false)
		return false;
	return true;
}

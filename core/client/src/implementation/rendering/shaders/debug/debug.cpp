// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.shaders.debug;

import :engine;

using namespace pragma;

decltype(ShaderDebug::VERTEX_BINDING_VERTEX) ShaderDebug::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderDebug::VERTEX_ATTRIBUTE_POSITION) ShaderDebug::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat};

ShaderDebug::ShaderDebug(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader) : ShaderScene(context, identifier, vsShader, fsShader)
{
	SetPipelineCount(math::to_integral(Pipeline::Count) * math::to_integral(PipelineType::Count));
}
ShaderDebug::ShaderDebug(prosper::IPrContext &context, const std::string &identifier) : ShaderDebug(context, identifier, "programs/debug/debug", "programs/debug/debug") {}

bool ShaderDebug::ShouldInitializePipeline(uint32_t pipelineIdx) { return true; }

void ShaderDebug::InitializeShaderResources()
{
	ShaderScene::InitializeShaderResources();

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::VertexBit);
}

void ShaderDebug::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	auto basePipelineIdx = pipelineIdx % math::to_integral(Pipeline::Count);
	auto pipelineType = static_cast<PipelineType>(pipelineIdx / math::to_integral(Pipeline::Count));

	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
	pipelineInfo.ToggleDepthBias(true, 1.f, 0.f, 0.f);
	pipelineInfo.ToggleDynamicStates(true, {prosper::DynamicState::DepthBias});

	switch(pipelineType) {
	case PipelineType::NoDepth:
		pipelineInfo.ToggleDepthTest(true, prosper::CompareOp::Always);
		break;
	}

	VERTEX_BINDING_VERTEX.stride = std::numeric_limits<decltype(VERTEX_BINDING_VERTEX.stride)>::max();
	switch(static_cast<Pipeline>(basePipelineIdx)) {
	case Pipeline::Triangle:
		break;
	case Pipeline::Line:
		pipelineInfo.ToggleDynamicStates(true, {prosper::DynamicState::LineWidth});
		pipelineInfo.SetPrimitiveTopology(prosper::PrimitiveTopology::LineList);
	case Pipeline::Wireframe:
		prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo, prosper::PolygonMode::Line);
		break;
	case Pipeline::LineStrip:
		prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo, prosper::PolygonMode::Line);
		pipelineInfo.SetPrimitiveTopology(prosper::PrimitiveTopology::LineStrip);
		pipelineInfo.ToggleDynamicStates(true, {prosper::DynamicState::LineWidth});
		break;
	case Pipeline::Vertex:
		VERTEX_BINDING_VERTEX.stride = sizeof(math::Vertex);
	case Pipeline::Point:
		prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo, prosper::PolygonMode::Point);
		pipelineInfo.SetPrimitiveTopology(prosper::PrimitiveTopology::PointList);
		break;
	}
}

bool ShaderDebug::RecordBeginDraw(prosper::ShaderBindState &bindState, Pipeline pipelineIdx) const { return ShaderGraphics::RecordBeginDraw(bindState, math::to_integral(pipelineIdx)) == true && bindState.commandBuffer.RecordSetDepthBias(1.f, 0.f, 0.f); }

bool ShaderDebug::RecordDraw(prosper::ShaderBindState &bindState, const std::vector<prosper::IBuffer *> &buffers, uint32_t vertexCount, const Mat4 &mvp, const Vector4 &color) const
{
	assert(vertexCount <= pragma::math::to_integral(pragma::GameLimits::MaxMeshVertices));
	if(vertexCount > math::to_integral(GameLimits::MaxMeshVertices)) {
		spdlog::error("Attempted to draw debug mesh with more than maximum ({}) amount of vertices!", math::to_integral(GameLimits::MaxMeshVertices));
		return false;
	}

	PushConstants pushConstants {mvp, color};
	if(RecordBindVertexBuffers(bindState, buffers) == false || RecordPushConstants(bindState, pushConstants) == false)
		return false;
	//pragma::get_cengine()->StartGPUTimer(GPUTimerEvent::DebugMesh); // prosper TODO
	auto r = ShaderGraphics::RecordDraw(bindState, vertexCount);
	//pragma::get_cengine()->StopGPUTimer(GPUTimerEvent::DebugMesh); // prosper TODO
	return r;
}

bool ShaderDebug::RecordDraw(prosper::ShaderBindState &bindState, prosper::IBuffer &vertexBuffer, uint32_t vertexCount, const Mat4 &mvp, const Vector4 &color) const { return RecordDraw(bindState, std::vector<prosper::IBuffer *> {&vertexBuffer}, vertexCount, mvp, color); }

/////////////////////

decltype(ShaderDebugTexture::VERTEX_BINDING_VERTEX) ShaderDebugTexture::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderDebugTexture::VERTEX_ATTRIBUTE_POSITION) ShaderDebugTexture::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32_SFloat};
decltype(ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE) ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURE",
  {prosper::DescriptorSetInfo::Binding {"TEXTURE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderDebugTexture::ShaderDebugTexture(prosper::IPrContext &context, const std::string &identifier) : ShaderScene(context, identifier, "programs/debug/debug_uv", "programs/debug/debug_texture") {}

void ShaderDebugTexture::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo, prosper::CullModeFlags::None);
	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
}
void ShaderDebugTexture::InitializeShaderResources()
{
	ShaderScene::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(0u, sizeof(ShaderDebug::PushConstants), prosper::ShaderStageFlags::VertexBit);
}
bool ShaderDebugTexture::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, const ShaderDebug::PushConstants &pushConstants) const
{
	auto buf = get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	return RecordBindVertexBuffer(bindState, *buf) && RecordBindDescriptorSet(bindState, descSetTexture, DESCRIPTOR_SET_TEXTURE.setIndex) && RecordPushConstants(bindState, pushConstants) && ShaderGraphics::RecordDraw(bindState, prosper::CommonBufferCache::GetSquareVertexCount());
}

/////////////////////

decltype(ShaderDebugVertexColor::VERTEX_BINDING_COLOR) ShaderDebugVertexColor::VERTEX_BINDING_COLOR = {prosper::VertexInputRate::Vertex};
decltype(ShaderDebugVertexColor::VERTEX_ATTRIBUTE_COLOR) ShaderDebugVertexColor::VERTEX_ATTRIBUTE_COLOR = {VERTEX_BINDING_COLOR, prosper::Format::R32G32B32A32_SFloat};

ShaderDebugVertexColor::ShaderDebugVertexColor(prosper::IPrContext &context, const std::string &identifier) : ShaderDebug(context, identifier, "programs/debug/debug_vertex_color", "programs/debug/debug") { SetBaseShader<ShaderDebug>(); }

void ShaderDebugVertexColor::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderDebug::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderDebugVertexColor::InitializeShaderResources()
{
	ShaderDebug::InitializeShaderResources();
	AddVertexAttribute(VERTEX_ATTRIBUTE_COLOR);
}

bool ShaderDebugVertexColor::RecordDraw(prosper::ShaderBindState &bindState, prosper::IBuffer &vertexBuffer, prosper::IBuffer &colorBuffer, uint32_t vertexCount, const Mat4 &modelMatrix) const
{
	return ShaderDebug::RecordDraw(bindState, std::vector<prosper::IBuffer *> {&vertexBuffer, &colorBuffer}, vertexCount, modelMatrix);
}

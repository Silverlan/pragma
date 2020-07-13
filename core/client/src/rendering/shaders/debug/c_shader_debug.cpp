/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/model/vertex.h>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderDebug::VERTEX_BINDING_VERTEX) ShaderDebug::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderDebug::VERTEX_ATTRIBUTE_POSITION) ShaderDebug::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32B32_SFloat};

ShaderDebug::ShaderDebug(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: ShaderScene(context,identifier,vsShader,fsShader)
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}
ShaderDebug::ShaderDebug(prosper::IPrContext &context,const std::string &identifier)
	: ShaderDebug(context,identifier,"debug/vs_debug","debug/fs_debug")
{}

bool ShaderDebug::ShouldInitializePipeline(uint32_t pipelineIdx) {return true;}

void ShaderDebug::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
	pipelineInfo.ToggleDepthBias(true,1.f,0.f,0.f);
	pipelineInfo.ToggleDynamicStates(true,{prosper::DynamicState::DepthBias});

	VERTEX_BINDING_VERTEX.stride = std::numeric_limits<decltype(VERTEX_BINDING_VERTEX.stride)>::max();
	switch(static_cast<Pipeline>(pipelineIdx))
	{
		case Pipeline::Triangle:
			break;
		case Pipeline::Line:
			pipelineInfo.ToggleDynamicStates(true,{prosper::DynamicState::LineWidth});
			pipelineInfo.SetPrimitiveTopology(prosper::PrimitiveTopology::LineList);
		case Pipeline::Wireframe:
			prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo,prosper::PolygonMode::Line);
			break;
		case Pipeline::LineStrip:
			prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo,prosper::PolygonMode::Line);
			pipelineInfo.SetPrimitiveTopology(prosper::PrimitiveTopology::LineStrip);
			pipelineInfo.ToggleDynamicStates(true,{prosper::DynamicState::LineWidth});
			break;
		case Pipeline::Vertex:
			VERTEX_BINDING_VERTEX.stride = sizeof(Vertex);
		case Pipeline::Point:
			prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo,prosper::PolygonMode::Point);
			pipelineInfo.SetPrimitiveTopology(prosper::PrimitiveTopology::PointList);
			break;
	}

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::VertexBit);
}

bool ShaderDebug::BeginDraw(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,Pipeline pipelineIdx)
{
	return ShaderGraphics::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx)) == true &&
		cmdBuffer->RecordSetDepthBias(1.f,0.f,0.f);
}

bool ShaderDebug::Draw(const std::vector<prosper::IBuffer*> &buffers,uint32_t vertexCount,const Mat4 &mvp,const Vector4 &color)
{
	assert(vertexCount <= umath::to_integral(GameLimits::MaxMeshVertices));
	if(vertexCount > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw debug mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return false;
	}

	PushConstants pushConstants {mvp,color};
	if(
		RecordBindVertexBuffers(buffers) == false ||
		RecordPushConstants(pushConstants) == false
	)
		return false;
	//c_engine->StartGPUTimer(GPUTimerEvent::DebugMesh); // prosper TODO
	auto r = RecordDraw(vertexCount);
	//c_engine->StopGPUTimer(GPUTimerEvent::DebugMesh); // prosper TODO
	return r;
}

bool ShaderDebug::Draw(prosper::IBuffer &vertexBuffer,uint32_t vertexCount,const Mat4 &mvp,const Vector4 &color)
{
	return Draw(std::vector<prosper::IBuffer*>{&vertexBuffer},vertexCount,mvp,color);
}

/////////////////////

decltype(ShaderDebugTexture::VERTEX_BINDING_VERTEX) ShaderDebugTexture::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderDebugTexture::VERTEX_ATTRIBUTE_POSITION) ShaderDebugTexture::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32_SFloat};
decltype(ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE) ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::DescriptorSetInfo::Binding {
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderDebugTexture::ShaderDebugTexture(prosper::IPrContext &context,const std::string &identifier)
	: ShaderScene(context,identifier,"debug/vs_debug_uv","debug/fs_debug_texture")
{}

void ShaderDebugTexture::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::None);
	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderDebug::PushConstants),prosper::ShaderStageFlags::VertexBit);
}
bool ShaderDebugTexture::Draw(prosper::IDescriptorSet &descSetTexture,const ShaderDebug::PushConstants &pushConstants)
{
	auto buf = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	return RecordBindVertexBuffer(*buf) &&
		RecordBindDescriptorSet(descSetTexture,DESCRIPTOR_SET_TEXTURE.setIndex) &&
		RecordPushConstants(pushConstants) &&
		RecordDraw(prosper::CommonBufferCache::GetSquareVertexCount());
}

/////////////////////

decltype(ShaderDebugVertexColor::VERTEX_BINDING_COLOR) ShaderDebugVertexColor::VERTEX_BINDING_COLOR = {prosper::VertexInputRate::Vertex};
decltype(ShaderDebugVertexColor::VERTEX_ATTRIBUTE_COLOR) ShaderDebugVertexColor::VERTEX_ATTRIBUTE_COLOR = {VERTEX_BINDING_COLOR,prosper::Format::R32G32B32A32_SFloat};

ShaderDebugVertexColor::ShaderDebugVertexColor(prosper::IPrContext &context,const std::string &identifier)
	: ShaderDebug(context,identifier,"debug/vs_debug_vertex_color","debug/fs_debug")
{
	SetBaseShader<ShaderDebug>();
}

void ShaderDebugVertexColor::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderDebug::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_COLOR);
}

bool ShaderDebugVertexColor::Draw(prosper::IBuffer &vertexBuffer,prosper::IBuffer &colorBuffer,uint32_t vertexCount,const Mat4 &modelMatrix)
{
	return ShaderDebug::Draw(std::vector<prosper::IBuffer*>{&vertexBuffer,&colorBuffer},vertexCount,modelMatrix);
}

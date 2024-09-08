/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_depth_to_rgb.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_context.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

decltype(ShaderDepthToRGB::VERTEX_BINDING_VERTEX) ShaderDepthToRGB::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderDepthToRGB::VERTEX_ATTRIBUTE_POSITION) ShaderDepthToRGB::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX, prosper::CommonBufferCache::GetSquareVertexFormat()};
decltype(ShaderDepthToRGB::VERTEX_ATTRIBUTE_UV) ShaderDepthToRGB::VERTEX_ATTRIBUTE_UV = {VERTEX_BINDING_VERTEX, prosper::CommonBufferCache::GetSquareUvFormat()};

decltype(ShaderDepthToRGB::DESCRIPTOR_SET) ShaderDepthToRGB::DESCRIPTOR_SET = {
  "TEXTURE",
  {prosper::DescriptorSetInfo::Binding {"DEPTH_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderDepthToRGB::ShaderDepthToRGB(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader) : ShaderGraphics(context, identifier, "programs/image/noop_uv", fsShader) {}
ShaderDepthToRGB::ShaderDepthToRGB(prosper::IPrContext &context, const std::string &identifier) : ShaderDepthToRGB(context, identifier, "programs/debug/depth_to_rgb") {}

void ShaderDepthToRGB::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo, true);
}
void ShaderDepthToRGB::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(VERTEX_ATTRIBUTE_UV);

	AttachPushConstantRange(0u, GetPushConstantSize(), prosper::ShaderStageFlags::FragmentBit);

	AddDescriptorSetGroup(DESCRIPTOR_SET);
}

template<class TPushConstants>
bool ShaderDepthToRGB::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetDepthTex, const TPushConstants &pushConstants) const
{
	return RecordBindVertexBuffers(bindState, {c_engine->GetRenderContext().GetCommonBufferCache().GetSquareVertexUvBuffer().get()}) == true && RecordBindDescriptorSet(bindState, descSetDepthTex) == true && RecordPushConstants(bindState, pushConstants) == true
	  && prosper::ShaderGraphics::RecordDraw(bindState, prosper::CommonBufferCache::GetSquareVertexCount()) == true;
}

bool ShaderDepthToRGB::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetDepthTex, float nearZ, float farZ, float contrastFactor) const { return RecordDraw(bindState, descSetDepthTex, PushConstants {nearZ, farZ, contrastFactor}); }

uint32_t ShaderDepthToRGB::GetPushConstantSize() const { return sizeof(PushConstants); }

//////////////////////

ShaderCubeDepthToRGB::ShaderCubeDepthToRGB(prosper::IPrContext &context, const std::string &identifier) : ShaderDepthToRGB(context, identifier, "programs/debug/cube_depth_to_rgb") {}

bool ShaderCubeDepthToRGB::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetDepthTex, float nearZ, float farZ, uint32_t cubeSide, float contrastFactor) const
{
	return ShaderDepthToRGB::RecordDraw(bindState, descSetDepthTex, PushConstants {{nearZ, farZ, contrastFactor}, static_cast<int32_t>(cubeSide)});
}

uint32_t ShaderCubeDepthToRGB::GetPushConstantSize() const { return sizeof(PushConstants); }

//////////////////////

ShaderCSMDepthToRGB::ShaderCSMDepthToRGB(prosper::IPrContext &context, const std::string &identifier) : ShaderDepthToRGB(context, identifier, "programs/debug/csm_depth_to_rgb") {}

bool ShaderCSMDepthToRGB::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetDepthTex, float nearZ, float farZ, uint32_t layer, float contrastFactor) const
{
	return ShaderDepthToRGB::RecordDraw(bindState, descSetDepthTex, PushConstants {{nearZ, farZ, contrastFactor}, static_cast<int32_t>(layer)});
}

uint32_t ShaderCSMDepthToRGB::GetPushConstantSize() const { return sizeof(PushConstants); }

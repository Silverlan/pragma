// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/game/game_limits.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>

module pragma.client.rendering.shaders;

import :debug_text;

using namespace pragma;

decltype(ShaderDebugText::DESCRIPTOR_SET_TEXTURE) ShaderDebugText::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURE",
  {prosper::DescriptorSetInfo::Binding {"TEXTURE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderDebugText::ShaderDebugText(prosper::IPrContext &context, const std::string &identifier) : ShaderDebug(context, identifier, "programs/debug/debug_text", "programs/debug/debug_text") { SetBaseShader<ShaderDebug>(); }

void ShaderDebugText::InitializeShaderResources()
{
	ShaderDebug::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
}
void ShaderDebugText::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderDebug::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

bool ShaderDebugText::RecordDraw(prosper::ShaderBindState &bindState, prosper::IBuffer &vertexBuffer, uint32_t vertexCount, prosper::IDescriptorSet &descSetTexture, const Mat4 &mvp, const Vector4 &color) const
{
	assert(vertexCount <= umath::to_integral(GameLimits::MaxMeshVertices));
	if(vertexCount > umath::to_integral(GameLimits::MaxMeshVertices)) {
		Con::cerr << "Attempted to draw debug mesh with more than maximum (" << umath::to_integral(GameLimits::MaxMeshVertices) << ") amount of vertices!" << Con::endl;
		return false;
	}
	return RecordBindDescriptorSet(bindState, descSetTexture) && ShaderDebug::RecordDraw(bindState, vertexBuffer, vertexCount, mvp, color);
}

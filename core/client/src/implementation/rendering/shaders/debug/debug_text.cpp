// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.shaders.debug_text;

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
	assert(vertexCount <= pragma::math::to_integral(pragma::GameLimits::MaxMeshVertices));
	if(vertexCount > math::to_integral(GameLimits::MaxMeshVertices)) {
		Con::CERR << "Attempted to draw debug mesh with more than maximum (" << math::to_integral(GameLimits::MaxMeshVertices) << ") amount of vertices!" << Con::endl;
		return false;
	}
	return RecordBindDescriptorSet(bindState, descSetTexture) && ShaderDebug::RecordDraw(bindState, vertexBuffer, vertexCount, mvp, color);
}

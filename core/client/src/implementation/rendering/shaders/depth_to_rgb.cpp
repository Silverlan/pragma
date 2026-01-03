// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.depth_to_rgb;

import :engine;

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
	return RecordBindVertexBuffers(bindState, {get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareVertexUvBuffer().get()}) == true && RecordBindDescriptorSet(bindState, descSetDepthTex) == true && RecordPushConstants(bindState, pushConstants) == true
	  && ShaderGraphics::RecordDraw(bindState, prosper::CommonBufferCache::GetSquareVertexCount()) == true;
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

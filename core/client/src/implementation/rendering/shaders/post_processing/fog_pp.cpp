// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.pp_fog;

using namespace pragma;

decltype(ShaderPPFog::DESCRIPTOR_SET_TEXTURE) ShaderPPFog::DESCRIPTOR_SET_TEXTURE = {shaderPPBase::get_descriptor_set_texture()};
decltype(ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER) ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER = {
  "DEPTH_BUFFER",
  {prosper::DescriptorSetInfo::Binding {"MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderPPFog::DESCRIPTOR_SET_SCENE) ShaderPPFog::DESCRIPTOR_SET_SCENE = {&ShaderScene::DESCRIPTOR_SET_SCENE};
decltype(ShaderPPFog::DESCRIPTOR_SET_FOG) ShaderPPFog::DESCRIPTOR_SET_FOG = {
  "FOG",
  {prosper::DescriptorSetInfo::Binding {"DATA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderPPFog::ShaderPPFog(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBase(context, identifier, "programs/post_processing/fog") { SetBaseShader<prosper::ShaderCopyImage>(); }

void ShaderPPFog::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
void ShaderPPFog::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();
	AddDefaultVertexAttributes();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_DEPTH_BUFFER);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_FOG);
}

bool ShaderPPFog::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetDepth, prosper::IDescriptorSet &descSetCamera, prosper::IDescriptorSet &descSetFog) const
{
	return RecordBindDescriptorSets(bindState, {&descSetDepth, &descSetCamera, &descSetFog}, DESCRIPTOR_SET_DEPTH_BUFFER.setIndex) && ShaderPPBase::RecordDraw(bindState, descSetTexture);
}

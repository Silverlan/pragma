/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_util.hpp>

using namespace pragma;

decltype(ShaderPPFog::DESCRIPTOR_SET_TEXTURE) ShaderPPFog::DESCRIPTOR_SET_TEXTURE = {ShaderPPBase::DESCRIPTOR_SET_TEXTURE};
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

// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <shader/prosper_shader_t.hpp>

module pragma.client.rendering.shaders;

import :pp_motion_blur;

using namespace pragma;

decltype(ShaderPPMotionBlur::DESCRIPTOR_SET_TEXTURE) ShaderPPMotionBlur::DESCRIPTOR_SET_TEXTURE = {ShaderPPBase::DESCRIPTOR_SET_TEXTURE};
decltype(ShaderPPMotionBlur::DESCRIPTOR_SET_TEXTURE_VELOCITY) ShaderPPMotionBlur::DESCRIPTOR_SET_TEXTURE_VELOCITY = {
  "VELOCITY",
  {prosper::DescriptorSetInfo::Binding {"TEXTURE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};

ShaderPPMotionBlur::ShaderPPMotionBlur(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBase(context, identifier, "programs/post_processing/motion_blur") { SetBaseShader<prosper::ShaderCopyImage>(); }
void ShaderPPMotionBlur::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderPPBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
void ShaderPPMotionBlur::InitializeShaderResources()
{
	ShaderPPBase::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE_VELOCITY);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}
bool ShaderPPMotionBlur::RecordDraw(prosper::ShaderBindState &bindState, const PushConstants &pushConstants, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetTextureVelocity) const
{
	return RecordBindDescriptorSets(bindState, {&descSetTextureVelocity}, DESCRIPTOR_SET_TEXTURE_VELOCITY.setIndex) && RecordPushConstants(bindState, pushConstants) && ShaderPPBase::RecordDraw(bindState, descSetTexture);
}

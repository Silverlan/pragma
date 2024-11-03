/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/c_settings.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <pragma/console/convars.h>
#include <prosper_util.hpp>
#include <shader/prosper_shader_copy_image.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

using namespace pragma;

decltype(ShaderPPHDR::DESCRIPTOR_SET_TEXTURE) ShaderPPHDR::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURES",
  {prosper::DescriptorSetInfo::Binding {"SCENE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"BLOOM", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"GLOW", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderPPHDR::RENDER_PASS_FORMAT) ShaderPPHDR::RENDER_PASS_FORMAT = prosper::Format::R8G8B8A8_UNorm;
decltype(ShaderPPHDR::RENDER_PASS_FORMAT_HDR) ShaderPPHDR::RENDER_PASS_FORMAT_HDR = prosper::Format::R16G16B16A16_SFloat;
ShaderPPHDR::ShaderPPHDR(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBase(context, identifier, "programs/post_processing/hdr")
{
	SetBaseShader<prosper::ShaderCopyImage>();
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}

void ShaderPPHDR::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	switch(static_cast<Pipeline>(pipelineIdx)) {
	case Pipeline::LDR:
		CreateCachedRenderPass<ShaderPPHDR>({{{RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::TransferSrcOptimal}}}, outRenderPass,
		  pipelineIdx);
		break;
	case Pipeline::HDR:
		CreateCachedRenderPass<ShaderPPHDR>({{{RENDER_PASS_FORMAT_HDR, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::TransferSrcOptimal}}}, outRenderPass,
		  pipelineIdx);
		break;
	}
}

void ShaderPPHDR::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	auto &settings = client->GetGameWorldShaderSettings();
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, 0u /* constantId */, static_cast<uint32_t>(settings.bloomEnabled));

	auto fxaaEnabled = (settings.fxaaEnabled && static_cast<Pipeline>(pipelineIdx) != Pipeline::HDR);
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, 1u /* constantId */, static_cast<uint32_t>(fxaaEnabled));

	static auto settingsGlowEnabled = false;
	auto glowEnabled = settingsGlowEnabled;
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, 2u /* constantId */, static_cast<uint32_t>(glowEnabled));
}

void ShaderPPHDR::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();
	AddDefaultVertexAttributes();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderPPHDR::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, pragma::rendering::ToneMapping toneMapping, float exposure, float bloomScale, float glowScale, bool flipVertically) const
{
	return RecordPushConstants(bindState, PushConstants {exposure, bloomScale, glowScale, toneMapping, flipVertically ? 1u : 0u}) && ShaderPPBase::RecordDraw(bindState, descSetTexture) == true;
}

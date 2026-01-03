// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.pp_hdr;

import :client_state;

using namespace pragma;

decltype(ShaderPPHDR::DESCRIPTOR_SET_TEXTURE) ShaderPPHDR::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURES",
  {prosper::DescriptorSetInfo::Binding {"SCENE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"BLOOM", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderPPHDR::RENDER_PASS_FORMAT) ShaderPPHDR::RENDER_PASS_FORMAT = prosper::Format::R8G8B8A8_UNorm;
decltype(ShaderPPHDR::RENDER_PASS_FORMAT_HDR) ShaderPPHDR::RENDER_PASS_FORMAT_HDR = prosper::Format::R16G16B16A16_SFloat;
ShaderPPHDR::ShaderPPHDR(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBase(context, identifier, "programs/post_processing/hdr")
{
	SetBaseShader<prosper::ShaderCopyImage>();
	SetPipelineCount(math::to_integral(Pipeline::Count));
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

	auto &settings = get_client_state()->GetGameWorldShaderSettings();
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, 0u /* constantId */, static_cast<uint32_t>(settings.bloomEnabled));

	auto fxaaEnabled = (settings.fxaaEnabled && static_cast<Pipeline>(pipelineIdx) != Pipeline::HDR);
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, 1u /* constantId */, static_cast<uint32_t>(fxaaEnabled));
}

void ShaderPPHDR::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();
	AddDefaultVertexAttributes();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderPPHDR::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, rendering::ToneMapping toneMapping, float exposure, float bloomScale, bool flipVertically) const
{
	return RecordPushConstants(bindState, PushConstants {exposure, bloomScale, toneMapping, flipVertically ? 1u : 0u}) && ShaderPPBase::RecordDraw(bindState, descSetTexture) == true;
}

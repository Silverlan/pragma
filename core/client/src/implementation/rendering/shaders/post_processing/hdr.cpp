// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.hdr;

using namespace pragma;

ShaderHDR::ShaderHDR(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing(context, identifier, "programs/post_processing/hdr") { SetBaseShader<prosper::ShaderCopyImage>(); }

void ShaderHDR::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderHDR::InitializeShaderResources()
{
	ShaderBaseImageProcessing::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderHDR::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, float exposure) const { return RecordPushConstants(bindState, PushConstants {exposure}) && ShaderBaseImageProcessing::RecordDraw(bindState, descSetTexture); }

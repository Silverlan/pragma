// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

module pragma.client;

import :rendering.shaders.brdf_convolution;

import :engine;


using namespace pragma;

ShaderBRDFConvolution::ShaderBRDFConvolution(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/lighting/brdf_convolution"} {}
void ShaderBRDFConvolution::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderBRDFConvolution>({{prosper::util::RenderPassCreateInfo::AttachmentInfo {prosper::Format::R16G16_SFloat}}}, outRenderPass, pipelineIdx);
}
void ShaderBRDFConvolution::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
void ShaderBRDFConvolution::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddDefaultVertexAttributes();
}
std::shared_ptr<prosper::Texture> ShaderBRDFConvolution::CreateBRDFConvolutionMap(uint32_t resolution)
{
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.format = prosper::Format::R16G16_SFloat;
	createInfo.width = resolution;
	createInfo.height = resolution;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.tiling = prosper::ImageTiling::Optimal;
	createInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit;
	auto img = pragma::get_cengine()->GetRenderContext().CreateImage(createInfo);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.minFilter = prosper::Filter::Linear;
	samplerCreateInfo.magFilter = prosper::Filter::Linear;
	auto tex = pragma::get_cengine()->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	auto rt = pragma::get_cengine()->GetRenderContext().CreateRenderTarget({tex}, GetRenderPass());

	auto vertBuffer = pragma::get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	auto uvBuffer = pragma::get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareUvBuffer();
	auto &setupCmd = pragma::get_cengine()->GetSetupCommandBuffer();
	auto success = false;
	if(setupCmd->RecordBeginRenderPass(*rt)) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState)) {
			if(RecordBindVertexBuffers(bindState, {vertBuffer.get(), uvBuffer.get()}) && prosper::ShaderGraphics::RecordDraw(bindState, prosper::CommonBufferCache::GetSquareVertexCount()))
				success = true;
		}
		setupCmd->RecordEndRenderPass();
	}
	pragma::get_cengine()->FlushSetupCommandBuffer();
	return success ? tex : nullptr;
}

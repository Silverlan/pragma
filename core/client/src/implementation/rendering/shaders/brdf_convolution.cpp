// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

module pragma.client.rendering.shaders;

import :brdf_convolution;

extern CEngine *c_engine;

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
	auto img = c_engine->GetRenderContext().CreateImage(createInfo);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.minFilter = prosper::Filter::Linear;
	samplerCreateInfo.magFilter = prosper::Filter::Linear;
	auto tex = c_engine->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	auto rt = c_engine->GetRenderContext().CreateRenderTarget({tex}, GetRenderPass());

	auto vertBuffer = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	auto uvBuffer = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareUvBuffer();
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	auto success = false;
	if(setupCmd->RecordBeginRenderPass(*rt)) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState)) {
			if(RecordBindVertexBuffers(bindState, {vertBuffer.get(), uvBuffer.get()}) && prosper::ShaderGraphics::RecordDraw(bindState, prosper::CommonBufferCache::GetSquareVertexCount()))
				success = true;
		}
		setupCmd->RecordEndRenderPass();
	}
	c_engine->FlushSetupCommandBuffer();
	return success ? tex : nullptr;
}

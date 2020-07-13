/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_brdf_convolution.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;


ShaderBRDFConvolution::ShaderBRDFConvolution(prosper::IPrContext &context,const std::string &identifier)
	: ShaderBaseImageProcessing{context,identifier,"screen/fs_brdf_convolution.gls"}
{}
void ShaderBRDFConvolution::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderBRDFConvolution>({{prosper::util::RenderPassCreateInfo::AttachmentInfo{prosper::Format::R16G16_SFloat}}},outRenderPass,pipelineIdx);
}
void ShaderBRDFConvolution::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddDefaultVertexAttributes(pipelineInfo);
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
	auto tex = c_engine->GetRenderContext().CreateTexture({},*img,imgViewCreateInfo,samplerCreateInfo);
	auto rt = c_engine->GetRenderContext().CreateRenderTarget({tex},GetRenderPass());

	auto vertBuffer = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	auto uvBuffer = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareUvBuffer();
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	auto success = false;
	if(setupCmd->RecordBeginRenderPass(*rt))
	{
		if(BeginDraw(setupCmd))
		{
			if(
				RecordBindVertexBuffers({vertBuffer.get(),uvBuffer.get()}) &&
				RecordDraw(prosper::CommonBufferCache::GetSquareVertexCount())
			)
				success = true;
		}
		setupCmd->RecordEndRenderPass();
	}
	c_engine->FlushSetupCommandBuffer();
	return success ? tex : nullptr;
}


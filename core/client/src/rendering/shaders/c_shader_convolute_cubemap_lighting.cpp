/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/shaders/c_shader_convolute_cubemap_lighting.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

decltype(ShaderConvoluteCubemapLighting::DESCRIPTOR_SET_CUBEMAP_TEXTURE) ShaderConvoluteCubemapLighting::DESCRIPTOR_SET_CUBEMAP_TEXTURE = {
  "TEXTURE",
  {prosper::DescriptorSetInfo::Binding {"CUBEMAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderConvoluteCubemapLighting::ShaderConvoluteCubemapLighting(prosper::IPrContext &context, const std::string &identifier) : ShaderCubemap {context, identifier, "programs/lighting/convolute_cubemap_lighting"} {}

void ShaderConvoluteCubemapLighting::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderCubemap::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderConvoluteCubemapLighting::InitializeShaderResources()
{
	ShaderCubemap::InitializeShaderResources();

	AddDescriptorSetGroup(DESCRIPTOR_SET_CUBEMAP_TEXTURE);
}

void ShaderConvoluteCubemapLighting::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderConvoluteCubemapLighting>({{prosper::util::RenderPassCreateInfo::AttachmentInfo {prosper::Format::R16G16B16A16_SFloat}}}, outRenderPass, pipelineIdx);
}

std::shared_ptr<prosper::Texture> ShaderConvoluteCubemapLighting::ConvoluteCubemapLighting(prosper::Texture &cubemap, uint32_t resolution)
{
	auto &cubemapImg = cubemap.GetImage();
	auto w = resolution;
	auto h = resolution;
	auto rt = CreateCubeMapRenderTarget(w, h);

	// Shader input
	auto dsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_CUBEMAP_TEXTURE);
	dsg->GetDescriptorSet()->SetBindingTexture(cubemap, 0u);

	PushConstants pushConstants {};
	pushConstants.projection = GetProjectionMatrix(w / static_cast<float>(h));

	// Generate cube
	uint32_t numVerts;
	auto vertexBuffer = CreateCubeMesh(numVerts);

	// Shader execution
	auto success = true;
	// Note: Convoluting the lighting takes a long time, which triggers the TDR timeout.
	// So, instead, we render each triangle of the cube separately for each individual layer.
	for(uint8_t layerId = 0u; layerId < 6u; ++layerId) {
		for(uint32_t i = 0u; i < numVerts; i += 3) {
			auto &setupCmd = c_engine->GetSetupCommandBuffer();
			util::ScopeGuard sgCmd {[this]() { GetContext().FlushSetupCommandBuffer(); }};
			prosper::util::ImageSubresourceRange range {};
			range.baseArrayLayer = layerId;
			range.layerCount = 1u;
			auto &img = rt->GetTexture().GetImage();
			if(setupCmd->RecordImageBarrier(img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal, range) == false || setupCmd->RecordBeginRenderPass(*rt, layerId) == false) {
				success = false;
				goto endLoop;
			}
			prosper::ShaderBindState bindState {*setupCmd};
			if(RecordBeginDraw(bindState) == true) {
				pushConstants.view = GetViewMatrix(layerId);
				success = RecordPushConstants(bindState, pushConstants) && RecordBindDescriptorSet(bindState, *dsg->GetDescriptorSet()) && RecordBindVertexBuffer(bindState, *vertexBuffer) && RecordDraw(bindState, 3u, 1u, i);
				RecordEndDraw(bindState);
			}
			success = success && setupCmd->RecordEndRenderPass();
			success = success && setupCmd->RecordPostRenderPassImageBarrier(img, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, range);

			if(success == false)
				goto endLoop;
		}
	}
endLoop:
	return success ? rt->GetTexture().shared_from_this() : nullptr;
}

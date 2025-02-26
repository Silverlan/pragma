/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/shaders/c_shader_compute_irradiance_map_roughness.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_image_view.hpp>
#include <prosper_framebuffer.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

decltype(ShaderComputeIrradianceMapRoughness::DESCRIPTOR_SET_IRRADIANCE) ShaderComputeIrradianceMapRoughness::DESCRIPTOR_SET_IRRADIANCE = {
  "IRRADIANCE",
  {prosper::DescriptorSetInfo::Binding {"IRRADIANCE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderComputeIrradianceMapRoughness::DESCRIPTOR_SET_ROUGHNESS) ShaderComputeIrradianceMapRoughness::DESCRIPTOR_SET_ROUGHNESS = {
  "ROUGHNESS",
  {prosper::DescriptorSetInfo::Binding {"ROUGHNESS", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderComputeIrradianceMapRoughness::ShaderComputeIrradianceMapRoughness(prosper::IPrContext &context, const std::string &identifier) : ShaderCubemap {context, identifier, "programs/lighting/compute_irradiance_map_roughness"} {}

void ShaderComputeIrradianceMapRoughness::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderCubemap::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderComputeIrradianceMapRoughness::InitializeShaderResources()
{
	ShaderCubemap::InitializeShaderResources();

	AddDescriptorSetGroup(DESCRIPTOR_SET_IRRADIANCE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_ROUGHNESS);
}

void ShaderComputeIrradianceMapRoughness::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderComputeIrradianceMapRoughness>({{prosper::util::RenderPassCreateInfo::AttachmentInfo {prosper::Format::R16G16B16A16_SFloat}}}, outRenderPass, pipelineIdx);
}

std::shared_ptr<prosper::Texture> ShaderComputeIrradianceMapRoughness::ComputeRoughness(prosper::Texture &cubemap, uint32_t resolution)
{
	auto &cubemapImg = cubemap.GetImage();
	auto w = resolution;
	auto h = resolution;
	prosper::util::ImageCreateInfo::Flags flags = prosper::util::ImageCreateInfo::Flags::FullMipmapChain;
	auto img = CreateCubeMap(w, h, flags);

	constexpr uint8_t layerCount = 6u;
	constexpr uint8_t maxMipLevels = 5u;
	struct MipLevelFramebuffer {
		std::shared_ptr<prosper::IImageView> imageView;
		std::shared_ptr<prosper::IFramebuffer> framebuffer;
	};
	std::array<std::array<MipLevelFramebuffer, maxMipLevels>, layerCount> imgViews {};
	for(auto layerId = decltype(imgViews.size()) {0u}; layerId < imgViews.size(); ++layerId) {
		auto &layerViews = imgViews.at(layerId);
		for(auto mipLevel = decltype(layerViews.size()) {0u}; mipLevel < layerViews.size(); ++mipLevel) {
			auto &mipLevelFramebuffer = layerViews.at(mipLevel);
			prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
			imgViewCreateInfo.baseLayer = layerId;
			imgViewCreateInfo.baseMipmap = mipLevel;
			imgViewCreateInfo.format = img->GetFormat();
			imgViewCreateInfo.levelCount = 1u;
			imgViewCreateInfo.mipmapLevels = 1u;

			mipLevelFramebuffer.imageView = c_engine->GetRenderContext().CreateImageView(imgViewCreateInfo, *img);
			uint32_t wMipmap, hMipmap;
			prosper::util::calculate_mipmap_size(w, h, &wMipmap, &hMipmap, mipLevel);
			std::vector<prosper::IImageView *> imgViewAttachments {mipLevelFramebuffer.imageView.get()};
			mipLevelFramebuffer.framebuffer = c_engine->GetRenderContext().CreateFramebuffer(wMipmap, hMipmap, 1u, imgViewAttachments);
		}
	}

	// Shader input
	auto dsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_IRRADIANCE);
	dsg->GetDescriptorSet()->SetBindingTexture(cubemap, 0u);

	PushConstants pushConstants {};
	pushConstants.projection = GetProjectionMatrix(w / static_cast<float>(h));

	// Generate cube
	uint32_t numVerts;
	auto vertexBuffer = CreateCubeMesh(numVerts);

	// Roughness data buffer
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
	bufCreateInfo.size = sizeof(RoughnessData);
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	bufCreateInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	auto buf = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo);
	buf->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);

	auto dsgRoughness = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_ROUGHNESS);
	dsgRoughness->GetDescriptorSet()->SetBindingUniformBuffer(*buf, 0);

	// Shader execution
	auto success = true;
	// Note: Computing the roughness takes a long time, which triggers the TDR timeout.
	// So, instead, we render each triangle of the cube separately for each individual layer.
	auto rp = GetRenderPass();
	RoughnessData roughnessData {};
	roughnessData.resolution = resolution;
	for(uint8_t mipLevel = 0u; mipLevel < maxMipLevels; ++mipLevel) {
		roughnessData.roughness = static_cast<float>(mipLevel) / static_cast<float>(maxMipLevels - 1u);
		for(uint8_t layerId = 0u; layerId < layerCount; ++layerId) {
			for(uint32_t i = 0u; i < numVerts; i += 3) {
				auto &setupCmd = c_engine->GetSetupCommandBuffer();
				util::ScopeGuard sgCmd {[this]() { GetContext().FlushSetupCommandBuffer(); }};
				setupCmd->RecordUpdateBuffer(*buf, 0, roughnessData);
				setupCmd->RecordBufferBarrier(*buf, prosper::PipelineStageFlags::HostBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::HostWriteBit, prosper::AccessFlags::ShaderReadBit);

				prosper::util::ImageSubresourceRange range {};
				range.baseArrayLayer = layerId;
				range.layerCount = 1u;
				range.baseMipLevel = mipLevel;
				range.levelCount = 1u;
				auto &mipLevelFramebuffer = imgViews.at(layerId).at(mipLevel);
				auto &fb = *mipLevelFramebuffer.framebuffer;

				if(setupCmd->RecordImageBarrier(*img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal, range) == false || setupCmd->RecordBeginRenderPass(*img, *rp, fb) == false) {
					success = false;
					goto endLoop;
				}

				prosper::ShaderBindState bindState {*setupCmd};
				if(RecordBeginDrawViewport(bindState, fb.GetWidth(), fb.GetHeight()) == true) {
					pushConstants.view = GetViewMatrix(layerId);
					success = RecordPushConstants(bindState, pushConstants) && RecordBindDescriptorSets(bindState, {dsg->GetDescriptorSet(), dsgRoughness->GetDescriptorSet()}) && RecordBindVertexBuffer(bindState, *vertexBuffer) && RecordDraw(bindState, 3u, 1u, i);
					RecordEndDraw(bindState);
				}
				success = success && setupCmd->RecordEndRenderPass();
				success = success && setupCmd->RecordPostRenderPassImageBarrier(*img, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, range);

				if(success == false)
					goto endLoop;
			}
		}
	}
endLoop:
	if(success == false)
		return nullptr;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	InitializeSamplerCreateInfo(flags, samplerCreateInfo);
	prosper::util::TextureCreateInfo texCreateInfo {};
	InitializeTextureCreateInfo(texCreateInfo);
	auto tex = c_engine->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);
	return success ? tex : nullptr;
}

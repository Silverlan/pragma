/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_util.hpp>
#include <prosper_context.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <util_image_buffer.hpp>
#include <util_texture_info.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_texture.hpp>
#include <image/prosper_image.hpp>
#include <image/prosper_sampler.hpp>
#include <sharedutils/util_file.h>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>
#include <texturemanager/texture.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

decltype(pragma::ShaderComposeRMA::DESCRIPTOR_SET_TEXTURE) pragma::ShaderComposeRMA::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURES",
  {prosper::DescriptorSetInfo::Binding {"ROUGHNESS", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"METALNESS", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"AMBIENT_OCCLUSION", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
pragma::ShaderComposeRMA::ShaderComposeRMA(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/util/compose_rma"} {}

std::shared_ptr<prosper::IImage> pragma::ShaderComposeRMA::ComposeRMA(prosper::IPrContext &context, prosper::Texture *roughnessMap, prosper::Texture *metalnessMap, prosper::Texture *aoMap, Flags flags)
{
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::SampledBit;

	auto fGetWhiteTex = [&context]() -> prosper::Texture * {
		auto tex = static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager().LoadAsset("white");
		if(tex == nullptr)
			return nullptr;
		return tex->GetVkTexture().get();
	};

	if(roughnessMap == nullptr)
		roughnessMap = fGetWhiteTex();
	if(metalnessMap == nullptr)
		metalnessMap = fGetWhiteTex();
	if(aoMap == nullptr)
		aoMap = fGetWhiteTex();
	if(roughnessMap == nullptr || metalnessMap == nullptr || aoMap == nullptr)
		return nullptr;

	auto &imgRoughness = roughnessMap->GetImage();
	auto &imgMetalness = metalnessMap->GetImage();
	auto &imgAo = aoMap->GetImage();

	auto extents = imgRoughness.GetExtents();
	extents.width = std::max({extents.width, imgMetalness.GetExtents().width, imgAo.GetExtents().width});
	extents.height = std::max({extents.height, imgMetalness.GetExtents().height, imgAo.GetExtents().height});

	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;

	auto imgRMA = context.CreateImage(imgCreateInfo);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	auto texRMA = context.CreateTexture({}, *imgRMA, imgViewCreateInfo);
	auto rt = context.CreateRenderTarget({texRMA}, GetRenderPass());

	auto dsg = CreateDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE.setIndex);
	auto &ds = *dsg->GetDescriptorSet();
	ds.SetBindingTexture(*roughnessMap, umath::to_integral(TextureBinding::RoughnessMap));
	ds.SetBindingTexture(*metalnessMap, umath::to_integral(TextureBinding::MetalnessMap));
	ds.SetBindingTexture(*aoMap, umath::to_integral(TextureBinding::AmbientOcclusionMap));

	auto &setupCmd = context.GetSetupCommandBuffer();
	if(setupCmd->RecordBeginRenderPass(*rt)) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState)) {
			PushConstants pushConstants {};
			pushConstants.flags = flags;
			if(RecordPushConstants(bindState, pushConstants))
				RecordDraw(bindState, ds);
			RecordEndDraw(bindState);
		}
		setupCmd->RecordEndRenderPass();
	}
	context.FlushSetupCommandBuffer();

	return texRMA->GetImage().shared_from_this();
}
bool pragma::ShaderComposeRMA::InsertAmbientOcclusion(prosper::IPrContext &context, const std::string &rmaInputPath, prosper::IImage &aoImg, const std::string *optRmaOutputPath)
{
	auto &texManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager();
	auto rmaTexInfo = texManager.LoadAsset(rmaInputPath);
	if(rmaTexInfo == nullptr || rmaTexInfo->HasValidVkTexture() == false)
		return false;

	prosper::util::TextureCreateInfo texCreateInfo {};
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto aoTex = context.CreateTexture(texCreateInfo, aoImg, imgViewCreateInfo, samplerCreateInfo);

	auto texRMA = (rmaTexInfo->IsError() == false) ? rmaTexInfo->GetVkTexture() : nullptr;
	auto newRMA = ComposeRMA(context, texRMA.get(), texRMA.get(), aoTex.get());

	uimg::TextureInfo imgWriteInfo {};
	imgWriteInfo.alphaMode = uimg::TextureInfo::AlphaMode::None;
	imgWriteInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
	imgWriteInfo.flags = uimg::TextureInfo::Flags::GenerateMipmaps;
	imgWriteInfo.inputFormat = uimg::TextureInfo::InputFormat::R8G8B8A8_UInt;
	imgWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::ColorMap;

	auto rmaOutputPath = optRmaOutputPath ? *optRmaOutputPath : rmaInputPath;

	std::string materialsRootDir = "materials/";
	auto matName = materialsRootDir + rmaOutputPath;
	ufile::remove_extension_from_filename(matName);

	Con::cout << "Writing RMA texture file '" << rmaOutputPath << "'..." << Con::endl;
	// TODO: RMA should overwrite the existing one
	return c_game->SaveImage(*newRMA, "addons/converted/" + matName, imgWriteInfo);
}
bool pragma::ShaderComposeRMA::InsertAmbientOcclusion(prosper::IPrContext &context, const std::string &rmaInputPath, uimg::ImageBuffer &imgBuffer, const std::string *optRmaOutputPath)
{
	auto aoImg = context.CreateImage(imgBuffer);
	return InsertAmbientOcclusion(context, rmaInputPath, *aoImg, optRmaOutputPath);
}

void pragma::ShaderComposeRMA::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
}

void pragma::ShaderComposeRMA::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddDefaultVertexAttributes();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

void pragma::ShaderComposeRMA::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<pragma::ShaderComposeRMA>(
	  std::vector<prosper::util::RenderPassCreateInfo::AttachmentInfo> {
	    {prosper::Format::R8G8B8A8_UNorm} // RMA
	  },
	  outRenderPass, pipelineIdx);
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/debug/widebughdrbloom.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include <sharedutils/util_string.h>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(WIDebugHDRBloom, WIDebugHDRBloom);

WIDebugHDRBloom::WIDebugHDRBloom() : WITexturedRect() {}

WIDebugHDRBloom::~WIDebugHDRBloom()
{
	if(m_cbRenderHDRMap.IsValid())
		m_cbRenderHDRMap.Remove();
	m_renderTarget = nullptr;
}

void WIDebugHDRBloom::UpdateBloomImage()
{
	auto &drawCmd = c_engine->GetDrawCommandBuffer();
	auto *scene = c_game->GetRenderScene();
	auto *renderer = scene ? dynamic_cast<pragma::CRendererComponent *>(scene->GetRenderer()) : nullptr;
	if(renderer == nullptr)
		return;
	auto raster = renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>();
	if(raster.expired())
		return;
	auto &bloomTexture = raster->GetHDRInfo().bloomBlurRenderTarget->GetTexture();
	auto &imgSrc = bloomTexture.GetImage();
	auto &imgDst = m_renderTarget->GetTexture().GetImage();

	drawCmd->RecordImageBarrier(imgSrc, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
	drawCmd->RecordImageBarrier(imgDst, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
	drawCmd->RecordBlitImage({}, imgSrc, imgDst);
	drawCmd->RecordImageBarrier(imgSrc, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	drawCmd->RecordImageBarrier(imgDst, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
}

void WIDebugHDRBloom::DoUpdate()
{
	WITexturedRect::DoUpdate();
	if(c_game == nullptr)
		return;
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = GetWidth();
	imgCreateInfo.height = GetHeight();
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferDstBit;
	auto img = c_engine->GetRenderContext().CreateImage(imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = c_engine->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	prosper::util::RenderPassCreateInfo rpInfo {};
	rpInfo.attachments.push_back({img->GetFormat(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, img->GetSampleCount(), prosper::ImageLayout::ColorAttachmentOptimal});
	rpInfo.subPasses.push_back({prosper::util::RenderPassCreateInfo::SubPass {{0ull}}});
	auto rp = c_engine->GetRenderContext().CreateRenderPass(rpInfo);
	m_renderTarget = c_engine->GetRenderContext().CreateRenderTarget({tex}, rp, {});
	m_cbRenderHDRMap = c_game->AddCallback("PostRenderScenes", FunctionCallback<>::Create(std::bind(&WIDebugHDRBloom::UpdateBloomImage, this)));
	SetTexture(m_renderTarget->GetTexture());
}

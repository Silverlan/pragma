/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/debug/widebugglowbloom.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <sharedutils/util_string.h>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(WIDebugGlowBloom,WIDebugGlowBloom);

WIDebugGlowBloom::WIDebugGlowBloom()
	: WITexturedRect()
{}

WIDebugGlowBloom::~WIDebugGlowBloom()
{
	if(m_cbRenderHDRMap.IsValid())
		m_cbRenderHDRMap.Remove();
	m_renderTarget = nullptr;
}

void WIDebugGlowBloom::UpdateBloomImage()
{
#if 0
	auto &drawCmd = c_engine->GetDrawCommandBuffer();
	auto *scene = c_game->GetScene();
	auto *renderer = scene ? dynamic_cast<pragma::CRasterizationRendererComponent*>(scene->GetRenderer()) : nullptr;
	if(renderer == nullptr)
		return;
	auto &glowTexture = renderer->GetGlowInfo().renderTarget->GetTexture();
	auto &imgSrc = glowTexture.GetImage();
	auto &imgDst = m_renderTarget->GetTexture().GetImage();
		
	drawCmd->RecordImageBarrier(imgSrc,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal);
	drawCmd->RecordImageBarrier(imgDst,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);
	drawCmd->RecordBlitImage({},imgSrc,imgDst);
	drawCmd->RecordImageBarrier(imgSrc,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	drawCmd->RecordImageBarrier(imgDst,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
#endif
}

void WIDebugGlowBloom::DoUpdate()
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
	auto tex = c_engine->GetRenderContext().CreateTexture({},*img,imgViewCreateInfo,samplerCreateInfo);
	prosper::util::RenderPassCreateInfo rpInfo {};
	rpInfo.attachments.push_back({img->GetFormat(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::Load,prosper::AttachmentStoreOp::Store,img->GetSampleCount(),prosper::ImageLayout::ColorAttachmentOptimal});
	rpInfo.subPasses.push_back({prosper::util::RenderPassCreateInfo::SubPass{{0ull}}});
	auto rp = c_engine->GetRenderContext().CreateRenderPass(rpInfo);
	m_renderTarget = c_engine->GetRenderContext().CreateRenderTarget({tex},rp,{});
	m_cbRenderHDRMap = c_game->AddCallback("PostRenderScenes",FunctionCallback<>::Create(
		std::bind(&WIDebugGlowBloom::UpdateBloomImage,this)
	));
	SetTexture(m_renderTarget->GetTexture());
}

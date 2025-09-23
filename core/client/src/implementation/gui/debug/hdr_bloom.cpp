// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/lua/converters/gui_element_converter.hpp"
#include <sharedutils/util_string.h>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

module pragma.client;

import :gui.debug_hdr_bloom;

import :client_state;
import :engine;
import :entities.components;
import :game;


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
	auto &drawCmd = pragma::get_cengine()->GetDrawCommandBuffer();
	auto *scene = pragma::get_cgame()->GetRenderScene<pragma::CSceneComponent>();
	auto *renderer = scene ? dynamic_cast<pragma::CRendererComponent *>(scene->GetRenderer<pragma::CRendererComponent>()) : nullptr;
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
	if(pragma::get_cgame() == nullptr)
		return;
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = GetWidth();
	imgCreateInfo.height = GetHeight();
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferDstBit;
	auto img = pragma::get_cengine()->GetRenderContext().CreateImage(imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = pragma::get_cengine()->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	prosper::util::RenderPassCreateInfo rpInfo {};
	rpInfo.attachments.push_back({img->GetFormat(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, img->GetSampleCount(), prosper::ImageLayout::ColorAttachmentOptimal});
	rpInfo.subPasses.push_back({prosper::util::RenderPassCreateInfo::SubPass {{0ull}}});
	auto rp = pragma::get_cengine()->GetRenderContext().CreateRenderPass(rpInfo);
	m_renderTarget = pragma::get_cengine()->GetRenderContext().CreateRenderTarget({tex}, rp, {});
	m_cbRenderHDRMap = pragma::get_cgame()->AddCallback("PostRenderScenes", FunctionCallback<>::Create(std::bind(&WIDebugHDRBloom::UpdateBloomImage, this)));
	SetTexture(m_renderTarget->GetTexture());
}

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.debug_hdr_bloom;

import :client_state;
import :engine;
import :entities.components;
import :game;

pragma::gui::types::WIDebugHDRBloom::WIDebugHDRBloom() : WITexturedRect() {}

pragma::gui::types::WIDebugHDRBloom::~WIDebugHDRBloom()
{
	if(m_cbRenderHDRMap.IsValid())
		m_cbRenderHDRMap.Remove();
	m_renderTarget = nullptr;
}

void pragma::gui::types::WIDebugHDRBloom::UpdateBloomImage()
{
	auto &drawCmd = get_cengine()->GetDrawCommandBuffer();
	auto *scene = get_cgame()->GetRenderScene<CSceneComponent>();
	auto *renderer = scene ? dynamic_cast<CRendererComponent *>(scene->GetRenderer<CRendererComponent>()) : nullptr;
	if(renderer == nullptr)
		return;
	auto raster = renderer->GetEntity().GetComponent<CRasterizationRendererComponent>();
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

void pragma::gui::types::WIDebugHDRBloom::DoUpdate()
{
	WITexturedRect::DoUpdate();
	if(get_cgame() == nullptr)
		return;
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = GetWidth();
	imgCreateInfo.height = GetHeight();
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferDstBit;
	auto img = get_cengine()->GetRenderContext().CreateImage(imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = get_cengine()->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	prosper::util::RenderPassCreateInfo rpInfo {};
	rpInfo.attachments.push_back({img->GetFormat(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, img->GetSampleCount(), prosper::ImageLayout::ColorAttachmentOptimal});
	rpInfo.subPasses.push_back({prosper::util::RenderPassCreateInfo::SubPass {{0ull}}});
	auto rp = get_cengine()->GetRenderContext().CreateRenderPass(rpInfo);
	m_renderTarget = get_cengine()->GetRenderContext().CreateRenderTarget({tex}, rp, {});
	m_cbRenderHDRMap = get_cgame()->AddCallback("PostRenderScenes", FunctionCallback<>::Create(std::bind(&WIDebugHDRBloom::UpdateBloomImage, this)));
	SetTexture(m_renderTarget->GetTexture());
}

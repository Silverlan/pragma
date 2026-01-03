// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.debug_msaa_texture;

import :engine;

pragma::gui::types::WIDebugMSAATexture::WIDebugMSAATexture() : WIBase() {}

void pragma::gui::types::WIDebugMSAATexture::Initialize()
{
	m_hTextureRect = CreateChild<WITexturedRect>();
	m_hTextureRect->SetAutoAlignToParent(true);
}

void pragma::gui::types::WIDebugMSAATexture::SetTexture(prosper::Texture &texture)
{
	m_msaaTexture = texture.shared_from_this();
	UpdateResolvedTexture();
	if(m_hTextureRect.IsValid() && m_resolvedTexture != nullptr)
		static_cast<WITexturedRect *>(m_hTextureRect.get())->SetTexture(*m_resolvedTexture);
}

void pragma::gui::types::WIDebugMSAATexture::UpdateResolvedTexture()
{
	m_resolvedTexture = nullptr;
	if(m_msaaTexture == nullptr)
		return;
	if(m_msaaTexture->GetImage().GetSampleCount() == prosper::SampleCountFlags::e1Bit) {
		m_resolvedTexture = m_msaaTexture;
		static_cast<WITexturedRect &>(*m_hTextureRect.get()).SetTexture(*m_resolvedTexture);
		return;
	}
	if(m_msaaTexture->IsMSAATexture()) {
		m_resolvedTexture = static_cast<prosper::MSAATexture &>(*m_msaaTexture).GetResolvedTexture();
		// prosper TODO: Use custom shader
		return;
	}
	auto &msaaImg = m_msaaTexture->GetImage();
	auto extents = msaaImg.GetExtents();
	auto &context = get_cengine()->GetRenderContext();
	prosper::util::ImageCreateInfo resolvedImgCreateInfo {};
	resolvedImgCreateInfo.width = extents.width;
	resolvedImgCreateInfo.height = extents.height;
	resolvedImgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	resolvedImgCreateInfo.format = msaaImg.GetFormat();
	resolvedImgCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
	resolvedImgCreateInfo.usage = prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::SampledBit;
	auto resolvedImg = context.CreateImage(resolvedImgCreateInfo);
	prosper::util::SamplerCreateInfo resolvedSamplerCreateInfo {};
	m_resolvedTexture = context.CreateTexture({}, *resolvedImg, {}, resolvedSamplerCreateInfo);
}

void pragma::gui::types::WIDebugMSAATexture::SetShouldResolveImage(bool b)
{
	if(m_bShouldResolveImage == b)
		return;
	m_bShouldResolveImage = b;
	UpdateResolvedTexture();
}

void pragma::gui::types::WIDebugMSAATexture::DoUpdate()
{
	WIBase::DoUpdate();
	if(!m_hTextureRect.IsValid())
		return;
	auto &context = get_cengine()->GetRenderContext();
	// TODO
}

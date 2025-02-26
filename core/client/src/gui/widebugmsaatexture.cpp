/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/gui/widebugmsaatexture.hpp"
#include <image/prosper_msaa_texture.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(widebugmsaatexture, WIDebugMSAATexture);

WIDebugMSAATexture::WIDebugMSAATexture() : WIBase() {}

void WIDebugMSAATexture::Initialize()
{
	m_hTextureRect = CreateChild<WITexturedRect>();
	m_hTextureRect->SetAutoAlignToParent(true);
}

void WIDebugMSAATexture::SetTexture(prosper::Texture &texture)
{
	m_msaaTexture = texture.shared_from_this();
	UpdateResolvedTexture();
	if(m_hTextureRect.IsValid() && m_resolvedTexture != nullptr)
		static_cast<WITexturedRect *>(m_hTextureRect.get())->SetTexture(*m_resolvedTexture);
}

void WIDebugMSAATexture::UpdateResolvedTexture()
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
	auto &context = c_engine->GetRenderContext();
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

void WIDebugMSAATexture::SetShouldResolveImage(bool b)
{
	if(m_bShouldResolveImage == b)
		return;
	m_bShouldResolveImage = b;
	UpdateResolvedTexture();
}

void WIDebugMSAATexture::DoUpdate()
{
	WIBase::DoUpdate();
	if(!m_hTextureRect.IsValid())
		return;
	auto &context = c_engine->GetRenderContext();
	// TODO
}

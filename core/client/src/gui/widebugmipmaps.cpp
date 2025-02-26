/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/gui/widebugmipmaps.h"
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_image_view.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(WIDebugMipMaps, WIDebugMipMaps);

WIDebugMipMaps::WIDebugMipMaps() : WIBase() {}

void WIDebugMipMaps::Initialize() { WIBase::Initialize(); }

void WIDebugMipMaps::SetTexture(const std::shared_ptr<prosper::Texture> &texture)
{
	for(auto &hTex : m_hTextures) {
		if(hTex.IsValid())
			hTex->Remove();
	}
	m_hTextures.clear();
	m_textures.clear();

	auto &wgui = WGUI::GetInstance();
	auto &context = wgui.GetContext();

	uint32_t width = 128;
	uint32_t height = 128;

	auto wContext = context.GetWindowWidth();

	auto &img = texture->GetImage();
	auto *imgView = texture->GetImageView();
	auto mipmapLevels = img.GetMipmapCount();
	auto keepSize = (mipmapLevels == 1);
	if(keepSize) {
		width = GetWidth();
		height = GetHeight();
	}
	m_hTextures.reserve(mipmapLevels);
	m_textures.reserve(mipmapLevels);
	uint32_t xOffset = 0;
	uint32_t hOffset = 0;
	std::vector<std::shared_ptr<prosper::Texture>> mipTextures;
	mipTextures.reserve(mipmapLevels);
	for(auto mipmap = decltype(mipmapLevels) {0}; mipmap < mipmapLevels; ++mipmap) {
		if(xOffset >= wContext) {
			xOffset = 0;
			hOffset += height;
		}
		auto hRect = CreateChild<WITexturedRect>();
		auto &r = static_cast<WITexturedRect &>(*hRect.get());

		auto extents = img.GetExtents(mipmap);
		prosper::util::ImageCreateInfo imgCreateInfo {};
		imgCreateInfo.width = extents.width;
		imgCreateInfo.height = extents.height;
		imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
		imgCreateInfo.usage = prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::SampledBit;
		imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
		auto dstImg = context.CreateImage(imgCreateInfo);
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		if(imgView) {
			auto swizzle = imgView->GetSwizzleArray();
			imgViewCreateInfo.swizzleRed = swizzle[0];
			imgViewCreateInfo.swizzleGreen = swizzle[1];
			imgViewCreateInfo.swizzleBlue = swizzle[2];
			imgViewCreateInfo.swizzleAlpha = swizzle[3];
		}
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		auto tex = context.CreateTexture({}, *dstImg, imgViewCreateInfo, samplerCreateInfo);
		mipTextures.push_back(tex);

		r.SetSize(width, height);
		r.SetTexture(*tex);
		r.SetPos(xOffset, hOffset);
		m_hTextures.push_back(hRect);
		m_textures.push_back(tex);
		xOffset += r.GetWidth();
	}
	auto cb = FunctionCallback<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>::Create(nullptr);
	static_cast<Callback<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>> *>(cb.get())->SetFunction([cb, &img, mipTextures](std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>> refDrawCmd) mutable {
		auto &drawCmd = *refDrawCmd.get();
		drawCmd.RecordImageBarrier(img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferSrcOptimal);

		for(auto mipLevel = decltype(mipTextures.size()) {0}; mipLevel < mipTextures.size(); ++mipLevel) {
			auto &mipTex = mipTextures.at(mipLevel);
			auto &dstImg = mipTex->GetImage();
			drawCmd.RecordImageBarrier(dstImg, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);

			prosper::util::BlitInfo blitInfo {};
			blitInfo.srcSubresourceLayer.mipLevel = mipLevel;
			blitInfo.dstSubresourceLayer.mipLevel = 0u;
			drawCmd.RecordBlitImage(blitInfo, img, dstImg);

			drawCmd.RecordImageBarrier(dstImg, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		}

		drawCmd.RecordImageBarrier(img, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		if(cb.IsValid())
			cb.Remove();
	});
	c_engine->AddCallback("DrawFrame", cb);
	if(!keepSize)
		SetSize(wContext, hOffset + height);
}

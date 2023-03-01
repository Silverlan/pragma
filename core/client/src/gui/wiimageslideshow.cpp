/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/wiimageslideshow.h"
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>
#include <prosper_util.hpp>
#include <image/prosper_texture.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_window.hpp>

extern DLLCLIENT CEngine *c_engine;

LINK_WGUI_TO_CLASS(WIImageSlideShow, WIImageSlideShow);

WIImageSlideShow::PreloadImage::PreloadImage() : ready(false), loading(false), image(-1) {}

WIImageSlideShow::WIImageSlideShow() : WIBase(), m_currentImg(-1) {}

void WIImageSlideShow::Initialize()
{
	WIBase::Initialize();
	EnableThinking();
	m_hImgNext = CreateChild<WITexturedRect>();
	m_hImgNext->SetAutoAlignToParent(true);

	m_hImgPrev = CreateChild<WITexturedRect>();
	m_hImgPrev->SetAutoAlignToParent(true);
	Update();
}

void WIImageSlideShow::SetColor(float r, float g, float b, float a)
{
	WIBase::SetColor(r, g, b, a);
	if(m_hImgNext.IsValid())
		m_hImgNext->SetColor(r, g, b, a);
	if(m_hImgPrev.IsValid())
		m_hImgPrev->SetColor(r, g, b, a);
}

void WIImageSlideShow::DoUpdate()
{
	WIBase::DoUpdate();
	//InitializeBlur(GetWidth(),GetHeight());
	//m_blurTexture.Initialize(*WGUI::GetContext(),m_texture,GetWidth(),GetHeight());
}

void WIImageSlideShow::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	ScheduleUpdate();
}

void WIImageSlideShow::Think()
{
	WIBase::Think();
	if(!IsVisible())
		return;
	m_tLastFade.Update();
	if((m_currentImg == -1 || m_tLastFade() > 18.f) && m_imgPreload.loading == false) {
		DisplayPreloadedImage();
		PreloadNextRandomShuffle();
	}
}

void WIImageSlideShow::DisplayPreloadedImage()
{
	auto &wiImgPreload = m_imgPreload;
	if(wiImgPreload.loading == true)
		return;
	if(!m_hImgPrev.IsValid() || !m_hImgNext.IsValid())
		return;
	m_tLastFade.Reset();
	m_currentImg = wiImgPreload.image;
	if(!wiImgPreload.ready)
		return;
	auto texPreload = wiImgPreload.texture->GetVkTexture();
	if(texPreload == nullptr)
		return;
	auto &imgPreload = texPreload->GetImage();
	auto lastTexture = (m_blurSet != nullptr) ? m_blurSet->GetFinalRenderTarget()->GetTexture().shared_from_this() : nullptr;

	auto &context = WGUI::GetInstance().GetContext();

	auto extents = imgPreload.GetExtents();
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit;
	createInfo.width = extents.width;
	createInfo.height = extents.height;
	createInfo.format = imgPreload.GetFormat();
	createInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;

	auto img = context.CreateImage(createInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = context.CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	prosper::util::RenderPassCreateInfo rpInfo {{img->GetFormat()}};
	auto rp = context.CreateRenderPass(rpInfo);
	auto rt = context.CreateRenderTarget({tex}, rp);
	rt->SetDebugName("img_slideshow_rt");

	m_blurSet = prosper::BlurSet::Create(context, rt, texPreload);
	if(m_blurSet == nullptr)
		return;

	m_lastTexture = lastTexture;
	if(wiImgPreload.texture == nullptr) {
		wiImgPreload.texture = nullptr;
		return;
	}

	auto &drawCmd = context.GetWindow().GetDrawCommandBuffer();
	try {
		prosper::util::record_blur_image(context, drawCmd, *m_blurSet,
		  {
		    Vector4(1.f, 1.f, 1.f, 1.f), 1.75f, /* Blur size */
		    9                                   /* Kernel size */
		  });
	}
	catch(const std::logic_error &e) {
		Con::cwar << "Unable to blur menu background image: '" << e.what() << "'!" << Con::endl;
	}
	texPreload = m_blurSet->GetFinalRenderTarget()->GetTexture().shared_from_this();

	auto *pImgPrev = static_cast<WITexturedRect *>(m_hImgPrev.get());
	auto *pImgNext = static_cast<WITexturedRect *>(m_hImgNext.get());

	if(m_lastTexture == nullptr)
		pImgPrev->ClearTexture();
	else
		pImgPrev->SetTexture(*m_lastTexture);

	pImgPrev->SetAlpha(1.f);
	pImgPrev->FadeOut(4.f);

	pImgNext->SetTexture(*texPreload);
	wiImgPreload.texture = nullptr;
}

void WIImageSlideShow::SetImages(const std::vector<std::string> &images)
{
	m_files = images;
	if(m_files.empty() == false || m_hImgNext.IsValid() == false)
		return;
	// No images available; Generate a black image and apply it
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.usage = prosper::ImageUsageFlags::SampledBit;
	createInfo.width = 1u;
	createInfo.height = 1u;
	createInfo.format = prosper::Format::R8G8B8A8_UNorm;
	createInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
	std::array<uint8_t, 4> px = {0u, 0u, 0u, std::numeric_limits<uint8_t>::max()};
	auto img = c_engine->GetRenderContext().CreateImage(createInfo, px.data());
	if(img != nullptr) {
		prosper::util::TextureCreateInfo texCreateInfo {};
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.addressModeU = samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::Repeat;
		auto tex = c_engine->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);
		if(tex != nullptr) {
			auto *pImgNext = static_cast<WITexturedRect *>(m_hImgNext.get());
			pImgNext->SetTexture(*tex);
		}
	}
}
void WIImageSlideShow::PreloadNextImage(Int32 img)
{
	auto &imgPreload = m_imgPreload;
	if(imgPreload.loading == true)
		return;
	auto f = m_files[img];
	imgPreload.loading = true;
	imgPreload.ready = false;
	imgPreload.image = img;

	auto &wgui = WGUI::GetInstance();
	auto &matManager = static_cast<msys::CMaterialManager &>(wgui.GetMaterialManager());
	auto &textureManager = matManager.GetTextureManager();
	auto hSlideShow = GetHandle();

	auto loadInfo = std::make_unique<msys::TextureLoadInfo>();
	loadInfo->flags |= util::AssetLoadFlags::AbsolutePath;
	loadInfo->onLoaded = [this, hSlideShow](util::Asset &asset) {
		if(!hSlideShow.IsValid())
			return;
		m_imgPreload.texture = msys::TextureManager::GetAssetObject(asset);
		m_imgPreload.ready = true;
		m_imgPreload.loading = false;
	};
	textureManager.PreloadAsset(f, std::move(loadInfo));
}

void WIImageSlideShow::DisplayNextImage()
{
	auto img = m_currentImg + 1;
	if(img >= m_files.size())
		img = 0;
	PreloadNextImage(Int32(img));
}

void WIImageSlideShow::PreloadNextRandomShuffle()
{
	if(m_randomShuffle.empty()) {
		m_randomShuffle.resize(m_files.size());
		for(size_t i = 0; i < m_files.size(); i++)
			m_randomShuffle.at(i) = i;
	}
	if(m_randomShuffle.empty())
		return;
	auto r = umath::random(0, CUInt32(m_randomShuffle.size() - 1));
	auto img = m_randomShuffle[r];
	m_randomShuffle.erase(m_randomShuffle.begin() + r);
	if(!m_randomShuffle.empty() && img == m_currentImg) {
		PreloadNextRandomShuffle();
		return;
	}
	PreloadNextImage(Int32(img));
}

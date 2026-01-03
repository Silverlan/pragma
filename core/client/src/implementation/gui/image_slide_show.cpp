// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.image_slide_show;
import :engine;

pragma::gui::types::WIImageSlideShow::PreloadImage::PreloadImage() : ready(false), loading(false), image(-1) {}

pragma::gui::types::WIImageSlideShow::WIImageSlideShow() : WIBase(), m_currentImg(-1) {}

void pragma::gui::types::WIImageSlideShow::Initialize()
{
	WIBase::Initialize();
	EnableThinking();
	m_hImgNext = CreateChild<WITexturedRect>();
	m_hImgNext->SetAutoAlignToParent(true);

	m_hImgPrev = CreateChild<WITexturedRect>();
	m_hImgPrev->SetAutoAlignToParent(true);
	Update();
}

void pragma::gui::types::WIImageSlideShow::SetColor(float r, float g, float b, float a)
{
	WIBase::SetColor(r, g, b, a);
	if(m_hImgNext.IsValid())
		m_hImgNext->SetColor(r, g, b, a);
	if(m_hImgPrev.IsValid())
		m_hImgPrev->SetColor(r, g, b, a);
}

void pragma::gui::types::WIImageSlideShow::DoUpdate()
{
	WIBase::DoUpdate();
	//InitializeBlur(GetWidth(),GetHeight());
	//m_blurTexture.Initialize(*WGUI::GetContext(),m_texture,GetWidth(),GetHeight());
}

void pragma::gui::types::WIImageSlideShow::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	ScheduleUpdate();
}

void pragma::gui::types::WIImageSlideShow::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	WIBase::Think(drawCmd);
	if(!IsVisible())
		return;
	m_tLastFade.Update();
	if((m_currentImg == -1 || m_tLastFade() > 18.f) && m_imgPreload.loading == false) {
		DisplayPreloadedImage();
		PreloadNextRandomShuffle();
	}
}

void pragma::gui::types::WIImageSlideShow::DisplayPreloadedImage()
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
		Con::CWAR << "Unable to blur menu background image: '" << e.what() << "'!" << Con::endl;
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

void pragma::gui::types::WIImageSlideShow::SetImages(const std::vector<std::string> &images)
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
	auto img = get_cengine()->GetRenderContext().CreateImage(createInfo, px.data());
	if(img != nullptr) {
		prosper::util::TextureCreateInfo texCreateInfo {};
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.addressModeU = samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::Repeat;
		auto tex = get_cengine()->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);
		if(tex != nullptr) {
			auto *pImgNext = static_cast<WITexturedRect *>(m_hImgNext.get());
			pImgNext->SetTexture(*tex);
		}
	}
}
void pragma::gui::types::WIImageSlideShow::PreloadNextImage(Int32 img)
{
	auto &imgPreload = m_imgPreload;
	if(imgPreload.loading == true)
		return;
	auto f = m_files[img];
	imgPreload.loading = true;
	imgPreload.ready = false;
	imgPreload.image = img;

	auto &wgui = WGUI::GetInstance();
	auto &matManager = static_cast<material::CMaterialManager &>(wgui.GetMaterialManager());
	auto &textureManager = matManager.GetTextureManager();
	auto hSlideShow = GetHandle();

	auto loadInfo = std::make_unique<material::TextureLoadInfo>();
	loadInfo->flags |= util::AssetLoadFlags::AbsolutePath;

	auto hThis = GetHandle();
	auto onLoaded = [this, hThis, hSlideShow](util::Asset *asset, util::AssetLoadResult result) {
		if(result != util::AssetLoadResult::Succeeded || !hThis.IsValid() || !hSlideShow.IsValid())
			return;
		m_imgPreload.texture = material::TextureManager::GetAssetObject(*asset);
		m_imgPreload.ready = true;
		m_imgPreload.loading = false;
	};

	auto preloadResult = textureManager.PreloadAsset(f, std::move(loadInfo));
	if(preloadResult.result == util::FileAssetManager::PreloadResult::Result::AlreadyLoaded) {
		auto *asset = textureManager.FindCachedAsset(f);
		if(asset)
			onLoaded(asset, util::AssetLoadResult::Succeeded);
	}
	else if(preloadResult.assetRequest != nullptr)
		preloadResult.assetRequest->AddCallback(onLoaded);
}

void pragma::gui::types::WIImageSlideShow::DisplayNextImage()
{
	auto img = m_currentImg + 1;
	if(img >= m_files.size())
		img = 0;
	PreloadNextImage(Int32(img));
}

void pragma::gui::types::WIImageSlideShow::PreloadNextRandomShuffle()
{
	if(m_randomShuffle.empty()) {
		m_randomShuffle.resize(m_files.size());
		for(size_t i = 0; i < m_files.size(); i++)
			m_randomShuffle.at(i) = i;
	}
	if(m_randomShuffle.empty())
		return;
	auto r = math::random(0, CUInt32(m_randomShuffle.size() - 1));
	auto img = m_randomShuffle[r];
	m_randomShuffle.erase(m_randomShuffle.begin() + r);
	if(!m_randomShuffle.empty() && img == m_currentImg) {
		PreloadNextRandomShuffle();
		return;
	}
	PreloadNextImage(Int32(img));
}

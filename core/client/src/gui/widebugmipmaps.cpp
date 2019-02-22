#include "stdafx_client.h"
#include "pragma/gui/widebugmipmaps.h"
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(WIDebugMipMaps,WIDebugMipMaps);

WIDebugMipMaps::WIDebugMipMaps()
	: WIBase()
{}

void WIDebugMipMaps::Initialize()
{
	WIBase::Initialize();
}

void WIDebugMipMaps::SetTexture(const std::shared_ptr<prosper::Texture> &texture)
{
	for(auto &hTex : m_hTextures)
	{
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
	auto mipmapLevels = img->GetMipmapCount();
	m_hTextures.reserve(mipmapLevels);
	m_textures.reserve(mipmapLevels);
	uint32_t xOffset = 0;
	uint32_t hOffset = 0;
	std::vector<std::shared_ptr<prosper::Texture>> mipTextures;
	mipTextures.reserve(mipmapLevels);
	for(auto mipmap=decltype(mipmapLevels){0};mipmap<mipmapLevels;++mipmap)
	{
		if(xOffset >= wContext)
		{
			xOffset = 0;
			hOffset += height;
		}
		auto hRect = CreateChild<WITexturedRect>();
		auto &r = *hRect.get<WITexturedRect>();

		auto &dev = context.GetDevice();
		prosper::util::ImageCreateInfo imgCreateInfo {};
		imgCreateInfo.width = width;
		imgCreateInfo.height = height;
		imgCreateInfo.format = Anvil::Format::R8G8B8A8_UNORM;
		imgCreateInfo.usage = Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
		imgCreateInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;
		auto dstImg = prosper::util::create_image(dev,imgCreateInfo);
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		auto tex = prosper::util::create_texture(dev,{},dstImg,&imgViewCreateInfo,&samplerCreateInfo);
		mipTextures.push_back(tex);

		r.SetSize(width,height);
		r.SetTexture(*tex);
		r.SetPos(xOffset,hOffset);
		m_hTextures.push_back(hRect);
		m_textures.push_back(tex);
		xOffset += r.GetWidth();
	}
	auto cb = FunctionCallback<void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>>::Create(nullptr);
	static_cast<Callback<void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>>*>(cb.get())->SetFunction([cb,img,mipTextures](std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>> refDrawCmd) mutable {
		auto &drawCmd = *refDrawCmd.get();
		prosper::util::record_image_barrier(*drawCmd,*(*img),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);

		for(auto mipLevel=decltype(mipTextures.size()){0};mipLevel<mipTextures.size();++mipLevel)
		{
			auto &mipTex = mipTextures.at(mipLevel);
			auto &dstImg = mipTex->GetImage();
			prosper::util::record_image_barrier(*drawCmd,*(*dstImg),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
			
			prosper::util::BlitInfo blitInfo {};
			blitInfo.srcSubresourceLayer.mip_level = mipLevel;
			blitInfo.dstSubresourceLayer.mip_level = 0u;
			prosper::util::record_blit_image(*drawCmd,blitInfo,*(*img),*(*dstImg));

			prosper::util::record_image_barrier(*drawCmd,*(*dstImg),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		}

		prosper::util::record_image_barrier(*drawCmd,*(*img),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		if(cb.IsValid())
			cb.Remove();
	});
	c_engine->AddCallback("DrawFrame",cb);
	SetSize(wContext,hOffset +height);
}

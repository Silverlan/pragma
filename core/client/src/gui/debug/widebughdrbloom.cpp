#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/debug/widebughdrbloom.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <sharedutils/util_string.h>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(WIDebugHDRBloom,WIDebugHDRBloom);

WIDebugHDRBloom::WIDebugHDRBloom()
	: WITexturedRect()
{}

WIDebugHDRBloom::~WIDebugHDRBloom()
{
	if(m_cbRenderHDRMap.IsValid())
		m_cbRenderHDRMap.Remove();
	m_renderTarget = nullptr;
}

void WIDebugHDRBloom::UpdateBloomImage()
{
	auto &drawCmd = c_engine->GetDrawCommandBuffer();
	auto &scene = c_game->GetScene();
	auto *renderer = dynamic_cast<pragma::rendering::RasterizationRenderer*>(scene->GetRenderer());
	if(renderer == nullptr)
		return;
	auto &bloomTexture = renderer->GetHDRInfo().bloomBlurRenderTarget->GetTexture();
	auto &imgSrc = bloomTexture->GetImage();
	auto &imgDst = m_renderTarget->GetTexture()->GetImage();
		
	prosper::util::record_image_barrier(**drawCmd,**imgSrc,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
	prosper::util::record_image_barrier(**drawCmd,**imgDst,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
	prosper::util::record_blit_image(**drawCmd,{},**imgSrc,**imgDst);
	prosper::util::record_image_barrier(**drawCmd,**imgSrc,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	prosper::util::record_image_barrier(**drawCmd,**imgDst,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
}

void WIDebugHDRBloom::Update()
{
	WITexturedRect::Update();
	if(c_game == nullptr)
		return;
	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = GetWidth();
	imgCreateInfo.height = GetHeight();
	imgCreateInfo.format = Anvil::Format::R8G8B8A8_UNORM;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
	auto img = prosper::util::create_image(dev,imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
	prosper::util::RenderPassCreateInfo rpInfo {};
	rpInfo.attachments.push_back({img->GetFormat(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,Anvil::AttachmentStoreOp::STORE,img->GetSampleCount(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL});
	rpInfo.subPasses.push_back({prosper::util::RenderPassCreateInfo::SubPass{{0ull}}});
	auto rp = prosper::util::create_render_pass(dev,rpInfo);
	m_renderTarget = prosper::util::create_render_target(dev,{tex},rp,{});
	m_cbRenderHDRMap = c_game->AddCallback("PostRenderScenes",FunctionCallback<>::Create(
		std::bind(&WIDebugHDRBloom::UpdateBloomImage,this)
	));
	SetTexture(*m_renderTarget->GetTexture());
}

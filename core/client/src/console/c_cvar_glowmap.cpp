#include "stdafx_client.h"
#include <wgui/wibase.h>
#include <wgui/wihandle.h>
#include <wgui/types/wirect.h>
#include <sharedutils/functioncallback.h>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <prosper_command_buffer.hpp>
#include "pragma/game/c_game_createguielement.h"

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
static const int dbgSize = 256; // Width / Height
struct DebugGlowMap
{
	WIHandle hGlowMap;
	CallbackHandle cbRenderGlowMap;
	CallbackHandle cbEndGame;
	std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
	DebugGlowMap(CallbackHandle &_cbEndGame)
		: renderTarget(nullptr)
	{
		auto &dev = c_engine->GetDevice();
		prosper::util::ImageCreateInfo imgCreateInfo {};
		imgCreateInfo.width = dbgSize;
		imgCreateInfo.height = dbgSize;
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
		renderTarget = prosper::util::create_render_target(dev,{tex},rp,{});
		cbEndGame = _cbEndGame;
		cbRenderGlowMap = c_game->AddCallback("PostRenderScenes",FunctionCallback<>::Create(
			std::bind(&DebugGlowMap::OnRender,this)
		));
		auto *rect = c_game->CreateGUIElement<WITexturedRect>();
		rect->SetSize(Vector2i(dbgSize,dbgSize));
		rect->SetTexture(*renderTarget->GetTexture());
		hGlowMap = rect->GetHandle();
	}
	~DebugGlowMap()
	{
		if(hGlowMap.IsValid())
			hGlowMap->Remove();
		if(cbRenderGlowMap.IsValid())
			cbRenderGlowMap.Remove();
		if(cbEndGame.IsValid())
			cbEndGame.Remove();
		renderTarget = nullptr;
	}
	void OnRender()
	{
		auto &drawCmd = c_engine->GetDrawCommandBuffer();
		auto &scene = c_game->GetScene();
		auto &glowTexture = scene->GetGlowInfo().renderTarget->GetTexture();
		auto &imgSrc = glowTexture->GetImage();
		auto &imgDst = renderTarget->GetTexture()->GetImage();
		
		prosper::util::record_image_barrier(**drawCmd,**imgSrc,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		prosper::util::record_image_barrier(**drawCmd,**imgDst,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		prosper::util::record_blit_image(**drawCmd,{},**imgSrc,**imgDst);
		prosper::util::record_image_barrier(**drawCmd,**imgSrc,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		prosper::util::record_image_barrier(**drawCmd,**imgDst,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	}
};

static std::unique_ptr<DebugGlowMap> s_glowMap = nullptr;

void CMD_debug_glow_bloom(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	if(c_game == nullptr || argv.empty())
		return;
	auto v = atoi(argv.front().c_str());
	if(v == 0)
	{
		s_glowMap = nullptr;
		return;
	}
	auto cbEndGame = c_game->AddCallback("EndGame",FunctionCallback<>::Create([]() {
		s_glowMap = nullptr;
	}));
	s_glowMap = std::make_unique<DebugGlowMap>(cbEndGame);
}
#pragma optimize("",on)

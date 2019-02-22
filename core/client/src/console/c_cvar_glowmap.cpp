#include "stdafx_client.h"
 // prosper TODO
#if 0
#include <wgui/wibase.h>
#include <wgui/wihandle.h>
#include <wgui/types/wirect.h>
#include <sharedutils/functioncallback.h>
#include "pragma/game/c_game_createguielement.h"

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static const int dbgSize = 256; // Width / Height
struct DebugGlowMap
{
	WIHandle hGlowMap;
	CallbackHandle cbRenderGlowMap;
	CallbackHandle cbEndGame;
	Vulkan::RenderTarget renderTarget;
	DebugGlowMap(CallbackHandle &_cbEndGame)
		: renderTarget(nullptr)
	{
		auto &context = c_engine->GetRenderContext();
		renderTarget = Vulkan::RenderTarget::Create(context,dbgSize,dbgSize,Anvil::Format::R8G8B8A8_UNORM,false,[](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
			info.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
		});
		cbEndGame = _cbEndGame;
		cbRenderGlowMap = c_game->AddCallback("PostRenderScene",FunctionCallback<>::Create(
			std::bind(&DebugGlowMap::OnRender,this)
		));
		auto *rect = c_game->CreateGUIElement<WITexturedRect>();
		rect->SetSize(Vector2i(dbgSize,dbgSize));
		rect->SetTexture(renderTarget->GetTexture());
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
		auto &context = c_engine->GetRenderContext();
		auto &drawCmd = context.GetDrawCmd();
		auto &scene = c_game->GetScene();
		auto &glowTexture = scene->GetGlowTexture();
		auto &imgSrc = glowTexture->GetImage();
		auto &imgDst = renderTarget->GetTexture()->GetImage();
		imgSrc->SetDrawLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		imgDst->SetDrawLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		drawCmd->BlitImage(imgSrc,imgDst);
	}
};

static std::unique_ptr<DebugGlowMap> s_glowMap = nullptr;
#endif

void CMD_debug_glow_bloom(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	/*if(c_game == nullptr || argv.empty())
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
	s_glowMap = std::make_unique<DebugGlowMap>(cbEndGame);*/ // prosper TODO
}

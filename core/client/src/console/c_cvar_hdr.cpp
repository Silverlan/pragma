#include "stdafx_client.h"
 // prosper TODO
#if 0
#include <wgui/wibase.h>
#include <wgui/wihandle.h>
#include <wgui/types/wirect.h>
#include <sharedutils/functioncallback.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static const int dbgSize = 256; // Width / Height
struct DebugHDRMap
{
	WIHandle hHDRMap;
	CallbackHandle cbRenderHDRMap;
	CallbackHandle cbEndGame;
	Vulkan::RenderTarget renderTarget;
	DebugHDRMap(CallbackHandle &_cbEndGame)
		: renderTarget(nullptr)
	{
		auto &context = c_engine->GetRenderContext();
		renderTarget = Vulkan::RenderTarget::Create(context,dbgSize,dbgSize,Anvil::Format::R8G8B8A8_UNORM,false,[](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
			info.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
		});
		cbEndGame = _cbEndGame;
		cbRenderHDRMap = c_game->AddCallback("PostRenderScene",FunctionCallback<>::Create(
			std::bind(&DebugHDRMap::OnRender,this)
		));
		auto *rect = c_game->CreateGUIElement<WITexturedRect>();
		rect->SetSize(Vector2i(dbgSize,dbgSize));
		rect->SetTexture(renderTarget->GetTexture());
		hHDRMap = rect->GetHandle();
	}
	~DebugHDRMap()
	{
		if(hHDRMap.IsValid())
			hHDRMap->Remove();
		if(cbRenderHDRMap.IsValid())
			cbRenderHDRMap.Remove();
		if(cbEndGame.IsValid())
			cbEndGame.Remove();
		renderTarget = nullptr;
	}
	void OnRender()
	{
		auto &context = c_engine->GetRenderContext();
		auto &drawCmd = context.GetDrawCmd();
		auto &scene = c_game->GetScene();
		auto &bloomTexture = scene->GetBloomTexture();
		auto &imgSrc = bloomTexture->GetImage();
		auto &imgDst = renderTarget->GetTexture()->GetImage();
		imgSrc->SetDrawLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		imgDst->SetDrawLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		drawCmd->BlitImage(imgSrc,imgDst);
	}
};

static std::unique_ptr<DebugHDRMap> s_hdrMap = nullptr;
#endif
void CMD_debug_hdr_bloom(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	/*if(c_game == nullptr || argv.empty())
		return;
	auto v = atoi(argv.front().c_str());
	if(v == 0)
	{
		s_hdrMap = nullptr;
		return;
	}
	auto cbEndGame = c_game->AddCallback("EndGame",FunctionCallback<>::Create([]() {
		s_hdrMap = nullptr;
	}));
	s_hdrMap = std::make_unique<DebugHDRMap>(cbEndGame);*/ // prosper TODO
}

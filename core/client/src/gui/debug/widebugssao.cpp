#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/debug/widebugssao.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <image/prosper_render_target.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(WIDebugSSAO,WIDebugSSAO);

WIDebugSSAO::WIDebugSSAO()
	: WITexturedRect()
{}

void WIDebugSSAO::SetUseBlurredSSAOImage(bool b) {m_bUseBlurVariant = b;}

void WIDebugSSAO::Update()
{
	WITexturedRect::Update();
	if(c_game == nullptr)
		return;
	auto &scene = c_game->GetScene();
	auto *renderer = dynamic_cast<pragma::rendering::RasterizationRenderer*>(scene->GetRenderer());
	if(renderer == nullptr)
		return;
	auto &ssaoInfo = renderer->GetSSAOInfo();
	if(m_bUseBlurVariant == false)
		SetTexture(*ssaoInfo.renderTarget->GetTexture());
	else
		SetTexture(*ssaoInfo.renderTargetBlur->GetTexture());
}

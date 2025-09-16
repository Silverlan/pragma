// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/lua/converters/gui_element_converter.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/game/c_game.h"
#include <image/prosper_render_target.hpp>

import pragma.client.entities.components;

import pragma.client.client_state;
module pragma.client.gui;

import :debug_ssao;

extern CEngine *c_engine;
extern ClientState *client;
extern CGame *c_game;

LINK_WGUI_TO_CLASS(WIDebugSSAO, WIDebugSSAO);

WIDebugSSAO::WIDebugSSAO() : WITexturedRect() {}

void WIDebugSSAO::SetUseBlurredSSAOImage(bool b) { m_bUseBlurVariant = b; }

void WIDebugSSAO::DoUpdate()
{
	WITexturedRect::DoUpdate();
	if(c_game == nullptr)
		return;
	auto *scene = c_game->GetScene<pragma::CSceneComponent>();
	auto *renderer = scene ? dynamic_cast<pragma::CRasterizationRendererComponent *>(scene->GetRenderer<pragma::CRendererComponent>()) : nullptr;
	if(renderer == nullptr)
		return;
	auto &ssaoInfo = renderer->GetSSAOInfo();
	if(m_bUseBlurVariant == false) {
		if(ssaoInfo.renderTarget)
			SetTexture(ssaoInfo.renderTarget->GetTexture());
	}
	else {
		if(ssaoInfo.renderTargetBlur)
			SetTexture(ssaoInfo.renderTargetBlur->GetTexture());
	}
}

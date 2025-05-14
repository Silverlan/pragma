/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/debug/widebugssao.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include <image/prosper_render_target.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(WIDebugSSAO, WIDebugSSAO);

WIDebugSSAO::WIDebugSSAO() : WITexturedRect() {}

void WIDebugSSAO::SetUseBlurredSSAOImage(bool b) { m_bUseBlurVariant = b; }

void WIDebugSSAO::DoUpdate()
{
	WITexturedRect::DoUpdate();
	if(c_game == nullptr)
		return;
	auto *scene = c_game->GetScene();
	auto *renderer = scene ? dynamic_cast<pragma::CRasterizationRendererComponent *>(scene->GetRenderer()) : nullptr;
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

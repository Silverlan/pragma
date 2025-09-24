// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/lua/converters/gui_element_converter.hpp"
#include <image/prosper_render_target.hpp>

module pragma.client;

import :entities.components;

import :gui.debug_ssao;

import :client_state;
import :engine;
import :game;

WIDebugSSAO::WIDebugSSAO() : WITexturedRect() {}

void WIDebugSSAO::SetUseBlurredSSAOImage(bool b) { m_bUseBlurVariant = b; }

void WIDebugSSAO::DoUpdate()
{
	WITexturedRect::DoUpdate();
	if(pragma::get_cgame() == nullptr)
		return;
	auto *scene = pragma::get_cgame()->GetScene<pragma::CSceneComponent>();
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

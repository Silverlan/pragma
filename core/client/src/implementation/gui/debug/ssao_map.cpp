// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components;

import :gui.debug_ssao;

import :client_state;
import :engine;
import :game;

pragma::gui::types::WIDebugSSAO::WIDebugSSAO() : WITexturedRect() {}

void pragma::gui::types::WIDebugSSAO::SetUseBlurredSSAOImage(bool b) { m_bUseBlurVariant = b; }

void pragma::gui::types::WIDebugSSAO::DoUpdate()
{
	WITexturedRect::DoUpdate();
	if(get_cgame() == nullptr)
		return;
	auto *scene = get_cgame()->GetScene<CSceneComponent>();
	auto *renderer = scene ? dynamic_cast<CRasterizationRendererComponent *>(scene->GetRenderer<CRendererComponent>()) : nullptr;
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

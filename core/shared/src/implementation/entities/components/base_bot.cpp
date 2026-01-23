// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_bot;

using namespace pragma;

void BaseBotComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	auto *pMdlComponent = static_cast<BaseModelComponent *>(ent.AddComponent("model").get());
	if(pMdlComponent != nullptr)
		pMdlComponent->SetModel("player/soldier.wmd");
}

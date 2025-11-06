// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

module pragma.client;

import :entities.components.func_button;
import :entities.components.render;

using namespace pragma;

void CButtonComponent::Initialize()
{
	BaseFuncButtonComponent::Initialize();
	auto pRenderComponent = static_cast<CBaseEntity &>(GetEntity()).GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::World);
}
void CButtonComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////

void CFuncButton::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CButtonComponent>();
}

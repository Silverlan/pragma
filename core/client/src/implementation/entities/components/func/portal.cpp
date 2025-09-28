// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client;


import :entities.components.func_portal;
import :engine;
import :entities.components.render;

using namespace pragma;

void CFuncPortalComponent::Initialize()
{
	BaseFuncPortalComponent::Initialize();
	auto pRenderComponent = static_cast<CBaseEntity &>(GetEntity()).GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::World);
}
void CFuncPortalComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void CFuncPortal::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFuncPortalComponent>();
}

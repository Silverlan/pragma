// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/c_engine.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.func_portal;

import pragma.client.entities.components.render;

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

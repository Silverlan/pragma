// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.func_button;

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_button, CFuncButton);

void CButtonComponent::Initialize()
{
	BaseFuncButtonComponent::Initialize();
	auto pRenderComponent = static_cast<CBaseEntity &>(GetEntity()).GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::World);
}
void CButtonComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////

void CFuncButton::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CButtonComponent>();
}

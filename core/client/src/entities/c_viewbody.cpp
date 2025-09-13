// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/parentinfo.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/components/logic_component.hpp>

import pragma.client.client_state;
import pragma.client.entities.components.render;
import pragma.client.entities.components.transform;

using namespace pragma;

extern ClientState *client;
extern CGame *c_game;

void CViewBodyComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent<CTransformComponent>();
	ent.AddComponent<CModelComponent>();
	ent.AddComponent<LogicComponent>(); // Logic component is needed for animations
	auto pRenderComponent = ent.AddComponent<CRenderComponent>();
	if(pRenderComponent.valid()) {
		pRenderComponent->AddToRenderGroup("firstperson");
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::View);
		pRenderComponent->SetCastShadows(false);
	}
	ent.AddComponent<CAnimatedComponent>();
}
void CViewBodyComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////

void CViewBody::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CViewBodyComponent>();
}

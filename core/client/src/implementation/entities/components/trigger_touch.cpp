// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/game/c_game.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.trigger_touch;

import pragma.client.entities.components.render;

using namespace pragma;

extern CGame *c_game;
void CTouchComponent::Initialize() { BaseTouchComponent::Initialize(); }
void CTouchComponent::OnEntitySpawn()
{
	BaseTouchComponent::OnEntitySpawn();
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	if(ent.IsClientsideOnly() == true) {
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent != nullptr)
			pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
	}
	auto pRenderComponent = static_cast<CBaseEntity &>(GetEntity()).GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::World);
}
void CTouchComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CTriggerTouch::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CTouchComponent>();
}

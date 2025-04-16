/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/trigger/c_trigger_touch.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(trigger_touch, CTriggerTouch);

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

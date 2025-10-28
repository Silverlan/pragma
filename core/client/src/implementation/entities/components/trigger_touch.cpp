// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_client.h"

module pragma.client;


import :entities.components.trigger_touch;
import :entities.components.render;

using namespace pragma;

void CTouchComponent::Initialize() { BaseTouchComponent::Initialize(); }
void CTouchComponent::OnEntitySpawn()
{
	BaseTouchComponent::OnEntitySpawn();
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	if(ent.IsClientsideOnly() == true) {
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent != nullptr)
			pPhysComponent->InitializePhysics(pragma::physics::PHYSICSTYPE::STATIC);
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

// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_client.h"

module pragma.client;


import :entities.components.func_physics;
import :entities.components.render;

using namespace pragma;

void CFuncPhysicsComponent::Initialize()
{
	BaseFuncPhysicsComponent::Initialize();
	auto pRenderComponent = static_cast<CBaseEntity &>(GetEntity()).GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::World);
}

void CFuncPhysicsComponent::ReceiveData(NetPacket &packet)
{
	m_kvMass = packet->Read<float>();
	m_kvSurfaceMaterial = packet->ReadString();
}

void CFuncPhysicsComponent::OnEntitySpawn() { BaseFuncPhysicsComponent::OnEntitySpawn(); }
void CFuncPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CFuncPhysics::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFuncPhysicsComponent>();
}

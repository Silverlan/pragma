// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/physics/movetypes.h"
#include <pragma/physics/physobj.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components;

import :func_physics;

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_physics, CFuncPhysics);

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

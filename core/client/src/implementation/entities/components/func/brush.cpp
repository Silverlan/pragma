// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/c_rendermode.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.func_brush;

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_brush, CFuncBrush);

void CBrushComponent::Initialize()
{
	BaseFuncBrushComponent::Initialize();
	auto pRenderComponent = static_cast<CBaseEntity &>(GetEntity()).GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::World);
}
void CBrushComponent::OnEntitySpawn()
{
	BaseFuncBrushComponent::OnEntitySpawn();
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	if(m_kvSolid) {
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent != nullptr)
			pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
		UpdateSurfaceMaterial(ent.GetNetworkState()->GetGameState());
	}
}
void CBrushComponent::ReceiveData(NetPacket &packet)
{
	m_kvSolid = packet->Read<bool>();
	m_kvSurfaceMaterial = packet->ReadString();
}
void CBrushComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

void CFuncBrush::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CBrushComponent>();
}

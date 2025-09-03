// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/game/s_game.h"
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.func.brush;

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_brush, FuncBrush);

extern DLLSERVER SGame *s_game;

void SBrushComponent::Initialize() { BaseFuncBrushComponent::Initialize(); }
void SBrushComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(!m_kvSolid)
		return;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
	UpdateSurfaceMaterial(s_game);
}
void SBrushComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<bool>(m_kvSolid);
	packet->WriteString(m_kvSurfaceMaterial);
}

void SBrushComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncBrush::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SBrushComponent>();
}

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/game/s_game.h"
#include <sharedutils/netpacket.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.func.soft_physics;

using namespace pragma;

void SFuncSoftPhysicsComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<PhysSoftBodyInfo>(m_softBodyInfo); }

void SFuncSoftPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

extern DLLSERVER SGame *s_game;

void FuncSoftPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFuncSoftPhysicsComponent>();
}

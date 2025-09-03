// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <pragma/audio/alsound.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/audio/alsound_type.h>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.func.kinematic;

import pragma.server.entities.components.point.path_node;

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_kinematic, FuncKinematic);

void SKinematicComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvFirstNode);
	packet->Write<float>(m_kvMoveSpeed);
	packet->Write<bool>(m_bMoving);
}

void SKinematicComponent::StartForward()
{
	BaseFuncKinematicComponent::StartForward();
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvStartForward, pragma::networking::Protocol::SlowReliable);
}

void SKinematicComponent::StartBackward()
{
	BaseFuncKinematicComponent::StartBackward();
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvStartBackward, pragma::networking::Protocol::SlowReliable);
}

void SKinematicComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncKinematic::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SKinematicComponent>();
}

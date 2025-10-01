// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <sharedutils/util_string.h>

module pragma.server.entities.components.func.kinematic;

import pragma.server.entities;
import pragma.server.entities.components.point.path_node;

using namespace pragma;

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

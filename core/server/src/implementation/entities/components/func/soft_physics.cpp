// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <sharedutils/netpacket.hpp>

module pragma.server.entities.components.func.soft_physics;

import pragma.server.game;

using namespace pragma;

void SFuncSoftPhysicsComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<PhysSoftBodyInfo>(m_softBodyInfo); }

void SFuncSoftPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

void FuncSoftPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFuncSoftPhysicsComponent>();
}

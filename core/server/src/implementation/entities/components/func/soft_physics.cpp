// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.func.soft_physics;

import :game;

using namespace pragma;

void SFuncSoftPhysicsComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<physics::PhysSoftBodyInfo>(m_softBodyInfo); }

void SFuncSoftPhysicsComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

void FuncSoftPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFuncSoftPhysicsComponent>();
}

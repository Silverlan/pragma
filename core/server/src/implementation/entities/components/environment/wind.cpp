// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.wind;

using namespace pragma;

void SWindComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<Vector3>(GetWindForce()); }

void SWindComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvWind::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SWindComponent>();
}

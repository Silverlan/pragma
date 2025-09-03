// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/netpacket.hpp>
#include "pragma/entities/s_baseentity.h"

module pragma.server.entities.components.skybox;

using namespace pragma;

LINK_ENTITY_TO_CLASS(skybox, Skybox);

void SSkyboxComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SSkyboxComponent::SetSkyAngles(const EulerAngles &ang)
{
	BaseSkyboxComponent::SetSkyAngles(ang);

	NetPacket p {};
	p->Write<EulerAngles>(ang);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetSkyAngles, p, pragma::networking::Protocol::SlowReliable);
}

void SSkyboxComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<EulerAngles>(m_skyAngles); }

void Skybox::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSkyboxComponent>();
}

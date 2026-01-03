// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.entity;

import :server_state;

using namespace pragma;

void SEntityComponentManager::OnComponentTypeRegistered(const ComponentInfo &componentInfo)
{
	if((componentInfo.flags & ComponentFlags::Networked) == ComponentFlags::None)
		return;
	NetPacket p {};
	p->Write<ComponentId>(componentInfo.id);
	p->WriteString(*componentInfo.name);
	ServerState::Get()->SendPacket(networking::net_messages::client::REGISTER_ENTITY_COMPONENT, p, networking::Protocol::SlowReliable);
}

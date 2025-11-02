// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.server;
import :entities.components.entity;

import :server_state;

using namespace pragma;

void SEntityComponentManager::OnComponentTypeRegistered(const ComponentInfo &componentInfo)
{
	if((componentInfo.flags & pragma::ComponentFlags::Networked) == pragma::ComponentFlags::None)
		return;
	NetPacket p {};
	p->Write<ComponentId>(componentInfo.id);
	p->WriteString(*componentInfo.name);
	ServerState::Get()->SendPacket("register_entity_component", p, pragma::networking::Protocol::SlowReliable);
}

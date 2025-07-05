// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/networking/enums.hpp>
#include <sharedutils/netpacket.hpp>

extern DLLSERVER ServerState *server;

using namespace pragma;

void SEntityComponentManager::OnComponentTypeRegistered(const ComponentInfo &componentInfo)
{
	if((componentInfo.flags & pragma::ComponentFlags::Networked) == pragma::ComponentFlags::None)
		return;
	NetPacket p {};
	p->Write<ComponentId>(componentInfo.id);
	p->WriteString(*componentInfo.name);
	server->SendPacket("register_entity_component", p, pragma::networking::Protocol::SlowReliable);
}

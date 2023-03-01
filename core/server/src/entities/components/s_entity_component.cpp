/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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

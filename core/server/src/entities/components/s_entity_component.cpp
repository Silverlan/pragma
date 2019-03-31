#include "stdafx_server.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <networkmanager/nwm_packet.h>

extern DLLSERVER ServerState *server;

using namespace pragma;

void SEntityComponentManager::OnComponentTypeRegistered(const ComponentInfo &componentInfo)
{
	if((componentInfo.flags &pragma::ComponentFlags::Networked) == pragma::ComponentFlags::None)
		return;
	NetPacket p {};
	p->Write<ComponentId>(componentInfo.id);
	p->WriteString(componentInfo.name);
	server->BroadcastTCP("register_entity_component",p);
}

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server.networking.util;

void nwm::write_unique_entity(NetPacket &packet, const pragma::ecs::BaseEntity *ent)
{
	packet->Write<util::Uuid>((ent != nullptr) ? ent->GetUuid() : util::Uuid {}); // 0 is an invalid index
}
void nwm::write_unique_entity(NetPacket &packet, const EntityHandle &hEnt) { write_unique_entity(packet, hEnt.get()); }

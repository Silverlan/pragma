// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :networking.util;

void pragma::networking::write_unique_entity(NetPacket &packet, const ecs::BaseEntity *ent)
{
	packet->Write<util::Uuid>((ent != nullptr) ? ent->GetUuid() : util::Uuid {}); // 0 is an invalid index
}
void pragma::networking::write_unique_entity(NetPacket &packet, const EntityHandle &hEnt) { write_unique_entity(packet, hEnt.get()); }

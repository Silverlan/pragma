// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:networking.util;

export import pragma.shared;

export namespace pragma::networking {
	DLLSERVER void write_unique_entity(NetPacket &packet, const ecs::BaseEntity *ent);
	DLLSERVER void write_unique_entity(NetPacket &packet, const EntityHandle &hEnt);
};

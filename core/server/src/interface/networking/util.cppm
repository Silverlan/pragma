// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"

export module pragma.server.networking.util;

export import pragma.shared;

export namespace nwm {
	DLLSERVER void write_unique_entity(NetPacket &packet, const pragma::ecs::BaseEntity *ent);
	DLLSERVER void write_unique_entity(NetPacket &packet, const EntityHandle &hEnt);
};

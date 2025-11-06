// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:networking.util;

export import :entities.base_entity;

export namespace nwm {
	DLLCLIENT CBaseEntity *read_unique_entity(NetPacket &packet);
	DLLCLIENT CallbackHandle read_unique_entity(NetPacket &packet, const std::function<void(pragma::ecs::BaseEntity *)> &onCreated);
};

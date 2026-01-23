// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:networking.util;

export import :entities.base_entity;

export namespace pragma::networking {
	DLLCLIENT ecs::CBaseEntity *read_unique_entity(NetPacket &packet);
	DLLCLIENT CallbackHandle read_unique_entity(NetPacket &packet, const std::function<void(ecs::BaseEntity *)> &onCreated);
};

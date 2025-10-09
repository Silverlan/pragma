// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "sharedutils/functioncallback.h"


export module pragma.client:networking.util;

import :entities.base_entity;

export namespace nwm {
	DLLCLIENT CBaseEntity *read_unique_entity(NetPacket &packet);
	DLLCLIENT CallbackHandle read_unique_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated);
};

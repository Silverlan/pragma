// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/networking/nwm_util.h>
#include <sharedutils/netpacket.hpp>

export module pragma.client.networking:util;

export namespace nwm {
	DLLCLIENT CBaseEntity *read_unique_entity(NetPacket &packet);
	DLLCLIENT CallbackHandle read_unique_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated);
};

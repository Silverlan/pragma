// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_NWM_UTIL_H__
#define __C_NWM_UTIL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/networking/nwm_util.h>

namespace nwm {
	DLLCLIENT CBaseEntity *read_unique_entity(NetPacket &packet);
	DLLCLIENT CallbackHandle read_unique_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated);
};

#endif

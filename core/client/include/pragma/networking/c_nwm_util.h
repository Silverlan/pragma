/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_NWM_UTIL_H__
#define __C_NWM_UTIL_H__

#include "pragma/clientdefinitions.h"
#include <pragma/networking/nwm_util.h>

namespace nwm {
	DLLCLIENT CBaseEntity *read_unique_entity(NetPacket &packet);
	DLLCLIENT CallbackHandle read_unique_entity(NetPacket &packet, const std::function<void(BaseEntity *)> &onCreated);
};

#endif

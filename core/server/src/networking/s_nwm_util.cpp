/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/networking/s_nwm_util.h"

void nwm::write_unique_entity(NetPacket &packet,const BaseEntity *ent)
{
	packet->Write<uint64_t>((ent != nullptr) ? ent->GetUniqueIndex() : (uint64_t)(0)); // 0 is an invalid index
}
void nwm::write_unique_entity(NetPacket &packet,const EntityHandle &hEnt) {write_unique_entity(packet,hEnt.get());}

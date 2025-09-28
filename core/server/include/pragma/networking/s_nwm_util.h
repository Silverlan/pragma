// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_NWM_UTIL_H__
#define __S_NWM_UTIL_H__


namespace nwm {
	DLLSERVER void write_unique_entity(NetPacket &packet, const BaseEntity *ent);
	DLLSERVER void write_unique_entity(NetPacket &packet, const EntityHandle &hEnt);
};

#endif

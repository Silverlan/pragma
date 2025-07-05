// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/networkdefinitions.h"
#include <sharedutils/netpacket.hpp>

DLLNETWORK bool NetIncludePacketID(NetworkState *state, std::string identifier, NetPacket &packet, NetPacket &packetNew);

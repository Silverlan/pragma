// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_UTIL_COMPONENT_STREAM_HPP__
#define __LUA_UTIL_COMPONENT_STREAM_HPP__

#include "pragma/networkdefinitions.h"

DLLNETWORK std::ostream &operator<<(std::ostream &stream, const pragma::BaseEntityComponent &component);

#endif

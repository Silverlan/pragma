// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LENTITY_COMPONWNT_LUA_HPP__
#define __LENTITY_COMPONWNT_LUA_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/sh_lua_component.hpp"
#include <sharedutils/util_weak_handle.hpp>

DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::BaseEntityComponent *hComponent);

#endif

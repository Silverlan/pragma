// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include "pragma/lua/core.hpp"

#include <string>

module pragma.shared;

import :entities.components.events.key_value;

using namespace pragma;

CEKeyValueData::CEKeyValueData(const std::string &key, const std::string &value) : key(key), value(value) {}
void CEKeyValueData::PushArguments(lua_State *l)
{
	Lua::PushString(l, key);
	Lua::PushString(l, value);
}

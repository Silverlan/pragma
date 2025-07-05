// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/lua/classes/lanimation.h"
#include "pragma/model/animation/play_animation_flags.hpp"

using namespace pragma;

CEKeyValueData::CEKeyValueData(const std::string &key, const std::string &value) : key(key), value(value) {}
void CEKeyValueData::PushArguments(lua_State *l)
{
	Lua::PushString(l, key);
	Lua::PushString(l, value);
}

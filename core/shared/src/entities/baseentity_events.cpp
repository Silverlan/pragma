/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/lua/classes/lanimation.h"
#include "pragma/model/animation/play_animation_flags.hpp"

using namespace pragma;

CEKeyValueData::CEKeyValueData(const std::string &key,const std::string &value)
	: key(key),value(value)
{}
void CEKeyValueData::PushArguments(lua_State *l)
{
	Lua::PushString(l,key);
	Lua::PushString(l,value);
}

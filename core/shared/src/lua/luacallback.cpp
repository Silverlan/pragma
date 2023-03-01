/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/luacallback.h"

LuaCallback::LuaCallback(const luabind::object &o)
    :
#ifdef CALLBACK_SANITY_CHECK_ENABLED
      TCallback(std::numeric_limits<size_t>::max()),
#else
      TCallback(),
#endif
      LuaFunction(o)
{
}
LuaCallback::~LuaCallback() {}

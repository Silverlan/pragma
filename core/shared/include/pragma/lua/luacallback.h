/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUACALLBACK_H__
#define __LUACALLBACK_H__

#include "pragma/networkdefinitions.h"
#include <sharedutils/functioncallback.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/luafunction.h"

class DLLNETWORK LuaCallback : public TCallback, public LuaFunction {
  public:
	LuaCallback(const luabind::object &o);
	virtual ~LuaCallback() override;
};

#endif

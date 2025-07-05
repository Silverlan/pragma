// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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

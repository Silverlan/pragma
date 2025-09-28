// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUACALLBACK_H__
#define __LUACALLBACK_H__

#include <sharedutils/functioncallback.h>

class DLLNETWORK LuaCallback : public TCallback, public LuaFunction {
  public:
	LuaCallback(const luabind::object &o);
	virtual ~LuaCallback() override;
};

#endif

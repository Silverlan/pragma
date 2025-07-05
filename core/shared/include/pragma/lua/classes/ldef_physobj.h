// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LDEF_PHYSOBJ_H__
#define __LDEF_PHYSOBJ_H__
#include "pragma/lua/ldefinitions.h"
#include "pragma/physics/physobj.h"
#define LUA_CHECK_PHYSOBJ(l, hPhys)                                                                                                                                                                                                                                                              \
	if(hPhys == nullptr) {                                                                                                                                                                                                                                                                       \
		lua_pushstring(l, "Attempted to use a NULL PhysObj");                                                                                                                                                                                                                                    \
		lua_error(l);                                                                                                                                                                                                                                                                            \
		return;                                                                                                                                                                                                                                                                                  \
	}

LUA_SETUP_HANDLE_CHECK(PhysObj, ::PhysObj, PhysObjHandle);
#endif

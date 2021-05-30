/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LDEF_PHYSOBJ_H__
#define __LDEF_PHYSOBJ_H__
#include "pragma/lua/ldefinitions.h"
#include "pragma/physics/physobj.h"
#define LUA_CHECK_PHYSOBJ(l,hPhys) \
	if(hPhys == nullptr) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL PhysObj"); \
		lua_error(l); \
		return; \
	}

LUA_SETUP_HANDLE_CHECK(PhysObj,::PhysObj,PhysObjHandle);
#endif
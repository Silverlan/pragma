/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LDEF_MODEL_H__
#define __LDEF_MODEL_H__
#include "pragma/lua/ldefinitions.h"
#include "pragma/model/model.h"

#define LUA_CHECK_MODEL(l,hModel) \
	if(hModel == nullptr) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL model"); \
		lua_error(l); \
		return; \
	}

//LUA_SETUP_HANDLE_CHECK(Model,::Model,ModelHandle);

#endif
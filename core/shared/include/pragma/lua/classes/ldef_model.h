// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LDEF_MODEL_H__
#define __LDEF_MODEL_H__
#include "pragma/lua/ldefinitions.h"
#include "pragma/model/model.h"

#define LUA_CHECK_MODEL(l, hModel)                                                                                                                                                                                                                                                               \
	if(hModel == nullptr) {                                                                                                                                                                                                                                                                      \
		lua_pushstring(l, "Attempted to use a NULL model");                                                                                                                                                                                                                                      \
		lua_error(l);                                                                                                                                                                                                                                                                            \
		return;                                                                                                                                                                                                                                                                                  \
	}

//LUA_SETUP_HANDLE_CHECK(Model,::Model,ModelHandle);

#endif

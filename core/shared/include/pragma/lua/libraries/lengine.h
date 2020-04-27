/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LENGINE_H__
#define __LENGINE_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

#undef LoadLibrary

namespace Lua
{
	namespace engine
	{
		DLLNETWORK int CreateLight(lua_State *l);
		DLLNETWORK int RemoveLights(lua_State *l);
		DLLNETWORK int CreateSprite(lua_State *l);
		DLLNETWORK int PrecacheModel_sv(lua_State *l);
		DLLNETWORK int LoadSoundScripts(lua_State *l);
		DLLNETWORK int get_model(lua_State *l);
		DLLNETWORK int LoadLibrary(lua_State *l);
		DLLNETWORK int GetTickCount(lua_State *l);
		DLLNETWORK int get_info(lua_State *l);
		DLLNETWORK int exit(lua_State *l);

		DLLNETWORK int32_t set_record_console_output(lua_State *l);
		DLLNETWORK int32_t poll_console_output(lua_State *l);
	};
};

#endif

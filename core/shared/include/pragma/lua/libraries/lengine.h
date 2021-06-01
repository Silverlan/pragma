/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LENGINE_H__
#define __LENGINE_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

#undef LoadLibrary

namespace Lua
{
	namespace engine
	{
		DLLNETWORK void PrecacheModel_sv(lua_State *l,const std::string &mdlName);
		DLLNETWORK void LoadSoundScripts(lua_State *l,const std::string &fileName,bool precache);
		DLLNETWORK void LoadSoundScripts(lua_State *l,const std::string &fileName);
		DLLNETWORK std::shared_ptr<Model> get_model(lua_State *l,const std::string &mdlName);
		DLLNETWORK int LoadLibrary(lua_State *l);
		DLLNETWORK int LibraryExists(lua_State *l);
		DLLNETWORK uint64_t GetTickCount();
		DLLNETWORK int get_info(lua_State *l);
		DLLNETWORK void exit();
		DLLNETWORK std::string get_working_directory();

		DLLNETWORK void set_record_console_output(bool record);
		DLLNETWORK int32_t poll_console_output(lua_State *l);
	};
};

#endif

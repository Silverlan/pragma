/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/lua/libraries/s_lengine.h"
#include "luasystem.h"
#include <pragma/serverstate/serverstate.h>

extern ServerState *server;
DLLSERVER int Lua_sv_engine_LoadMaterial(lua_State *l)
{
	std::string mat = Lua::CheckString(l,1);
	auto bReload = false;
	if(Lua::IsSet(l,2))
		bReload = Lua::CheckBool(l,2);
	Material *material = server->LoadMaterial(mat.c_str(),bReload);
	if(material == NULL)
		return 0;
	luabind::object(l,material).push(l);
	return 1;
}
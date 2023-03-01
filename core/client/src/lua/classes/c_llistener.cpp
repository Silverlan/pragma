/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_llistener.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/entities/c_listener.h"
#include "luasystem.h"
DLLCLIENT void Lua_Listener_GetGain(lua_State *l, ListenerHandle &hEnt)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CListener *listener = hEnt.get<CListener>();
	//Lua::PushNumber(l,listener->GetGain());
}

DLLCLIENT void Lua_Listener_SetGain(lua_State *l, ListenerHandle &hEnt, float gain)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CListener *listener = hEnt.get<CListener>();
	//listener->SetGain(gain);
}

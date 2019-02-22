#include "stdafx_client.h"
#include "pragma/lua/classes/c_llistener.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/libraries/c_listener.h"
#include "luasystem.h"
DLLCLIENT void Lua_Listener_GetGain(lua_State *l,ListenerHandle &hEnt)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CListener *listener = hEnt.get<CListener>();
	//Lua::PushNumber(l,listener->GetGain());
}

DLLCLIENT void Lua_Listener_SetGain(lua_State *l,ListenerHandle &hEnt,float gain)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CListener *listener = hEnt.get<CListener>();
	//listener->SetGain(gain);
}
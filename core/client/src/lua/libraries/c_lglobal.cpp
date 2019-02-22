#include "stdafx_client.h"
#include "pragma/lua/libraries/c_lglobal.h"
#include "luasystem.h"

extern DLLCENGINE CEngine *c_engine;

int Lua_ServerTime(lua_State *l)
{
	NetworkState *state = c_engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	Lua::PushNumber(l,game->ServerTime());
	return 1;
}

int Lua_FrameTime(lua_State *l)
{
	Lua::PushNumber(l,c_engine->GetDeltaFrameTime());
	return 1;
}

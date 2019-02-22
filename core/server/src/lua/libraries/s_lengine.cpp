#include "stdafx_server.h"
#include "pragma/lua/libraries/s_lengine.h"
#include "luasystem.h"
#include <pragma/serverstate/serverstate.h>
#include "pragma/lua/classes/s_lua_weapon.h"
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
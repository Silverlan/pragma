#include "stdafx_shared.h"
#include "pragma/lua/classes/lshaderinfo.h"
#include "luasystem.h"

DLLNETWORK void Lua_ShaderInfo_GetName(lua_State *l,ShaderInfo *shader)
{
	Lua::PushString(l,shader->GetIdentifier());
}
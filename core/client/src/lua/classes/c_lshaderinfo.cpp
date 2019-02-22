#include "stdafx_client.h"
#include "pragma/lua/classes/c_lshaderinfo.h"
#include "luasystem.h"
#include "shaderinfo.h"
DLLCLIENT void Lua_ShaderInfo_GetID(lua_State *l,ShaderInfo *shader)
{
	/*Lua::PushInt(l,shader->GetShaderID());*/ // Vulkan TODO
}

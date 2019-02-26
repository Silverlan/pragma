#include "stdafx_client.h"
#include "pragma/lua/classes/c_lshaderinfo.h"
#include "luasystem.h"
#include "sharedutils/util_shaderinfo.hpp"
DLLCLIENT void Lua_ShaderInfo_GetID(lua_State *l,util::ShaderInfo &shader)
{
	/*Lua::PushInt(l,shader->GetShaderID());*/ // Vulkan TODO
}

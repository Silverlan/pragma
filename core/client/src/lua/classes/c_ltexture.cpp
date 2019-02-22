#include "stdafx_client.h"
#include "pragma/lua/classes/c_ltexture.h"
#include "luasystem.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"

void Lua::Texture::GetWidth(lua_State *l,::Texture &tex)
{
	Lua::PushInt(l,tex.width);
}
void Lua::Texture::GetHeight(lua_State *l,::Texture &tex)
{
	Lua::PushInt(l,tex.height);
}
void Lua::Texture::GetVkTexture(lua_State *l,::Texture &tex)
{
	if(tex.texture == nullptr)
		return;
	Lua::Push(l,tex.texture);
}

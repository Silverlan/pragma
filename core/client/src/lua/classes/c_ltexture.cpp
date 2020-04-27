/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_ltexture.h"
#include "luasystem.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"

void Lua::Texture::GetWidth(lua_State *l,::Texture &tex)
{
	Lua::PushInt(l,tex.GetWidth());
}
void Lua::Texture::GetHeight(lua_State *l,::Texture &tex)
{
	Lua::PushInt(l,tex.GetHeight());
}
void Lua::Texture::GetVkTexture(lua_State *l,::Texture &tex)
{
	if(tex.HasValidVkTexture() == false)
		return;
	Lua::Push(l,tex.GetVkTexture());
}

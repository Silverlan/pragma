// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

module pragma.client;

import :scripting.lua.classes.texture;
std::shared_ptr<prosper::Texture> Lua::Texture::GetVkTexture(lua::State *l, msys::Texture &tex)
{
	if(tex.HasValidVkTexture() == false)
		return nullptr;
	return tex.GetVkTexture();
}

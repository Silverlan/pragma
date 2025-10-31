// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:scripting.lua.classes.texture;

export import pragma.cmaterialsystem;
export import pragma.prosper;

export namespace Lua {
	namespace Texture {
		DLLCLIENT std::shared_ptr<prosper::Texture> GetVkTexture(lua_State *l, msys::Texture &tex);
	};
};

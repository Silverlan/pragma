// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include "textureinfo.h"
#include <texturemanager/texture.h>

export module pragma.client:scripting.lua.classes.texture;
export namespace Lua {
	namespace Texture {
		DLLCLIENT std::shared_ptr<prosper::Texture> GetVkTexture(lua_State *l, ::Texture &tex);
	};
};

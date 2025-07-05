// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LTEXTURE_H__
#define __C_LTEXTURE_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <mathutil/glmutil.h>
#include "textureinfo.h"
#include <texturemanager/texture.h>

namespace Lua {
	namespace Texture {
		DLLCLIENT std::shared_ptr<prosper::Texture> GetVkTexture(lua_State *l, ::Texture &tex);
	};
};

#endif

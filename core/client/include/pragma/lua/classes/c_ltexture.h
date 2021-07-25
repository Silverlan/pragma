/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LTEXTURE_H__
#define __C_LTEXTURE_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <mathutil/glmutil.h>
#include "textureinfo.h"
#include <texturemanager/texture.h>

namespace Lua
{
	namespace Texture
	{
		DLLCLIENT std::shared_ptr<prosper::Texture> GetVkTexture(lua_State *l,::Texture &tex);
	};
};

#endif

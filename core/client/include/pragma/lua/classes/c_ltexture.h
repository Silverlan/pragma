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
		DLLCLIENT void GetWidth(lua_State *l,::Texture &tex);
		DLLCLIENT void GetHeight(lua_State *l,::Texture &tex);
		DLLCLIENT void GetVkTexture(lua_State *l,::Texture &tex);
	};
};

#endif

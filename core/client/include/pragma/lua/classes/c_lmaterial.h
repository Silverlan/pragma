/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LMATERIAL_H__
#define __C_LMATERIAL_H__

#include "pragma/clientdefinitions.h"
#include "matsysdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <texturemanager/texturemanager.h>
#include "pragma/lua/libraries/c_lua_vulkan.h"

class Material;
struct TextureInfo;
namespace Lua {
	namespace Material {
		namespace Client {
			DLLCLIENT void SetTexture(lua_State *l, ::Material *mat, const std::string &textureID, const std::string &tex);
			DLLCLIENT void SetTexture(lua_State *l, ::Material *mat, const std::string &textureID, ::Texture &tex);
			DLLCLIENT void SetTexture(lua_State *l, ::Material *mat, const std::string &textureID, Lua::Vulkan::Texture &hTex);
			DLLCLIENT void SetTexture(lua_State *l, ::Material *mat, const std::string &textureID, Lua::Vulkan::Texture &hTex, const std::string &name);
			DLLCLIENT void GetTexture(lua_State *l, ::Material *mat, const std::string &textureID);
			DLLCLIENT void InitializeShaderData(lua_State *l, ::Material *mat, bool reload);
			DLLCLIENT void InitializeShaderData(lua_State *l, ::Material *mat);
		};
	};
	namespace TextureInfo {
		DLLCLIENT std::shared_ptr<Texture> GetTexture(lua_State *l, ::TextureInfo *tex);
		DLLCLIENT std::pair<uint32_t, uint32_t> GetSize(lua_State *l, ::TextureInfo *tex);
		DLLCLIENT uint32_t GetWidth(lua_State *l, ::TextureInfo *tex);
		DLLCLIENT uint32_t GetHeight(lua_State *l, ::TextureInfo *tex);
	};
};

#endif

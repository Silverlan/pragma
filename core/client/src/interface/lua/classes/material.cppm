// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.material;

export import :scripting.lua.libraries.vulkan;
export import pragma.cmaterialsystem;

export namespace Lua {
	namespace Material {
		namespace Client {
			DLLCLIENT void SetTexture(lua::State *l, pragma::material::Material *mat, const std::string &textureID, const std::string &tex);
			DLLCLIENT void SetTexture(lua::State *l, pragma::material::Material *mat, const std::string &textureID, pragma::material::Texture &tex);
			DLLCLIENT void SetTexture(lua::State *l, pragma::material::Material *mat, const std::string &textureID, Vulkan::Texture &hTex);
			DLLCLIENT void SetTexture(lua::State *l, pragma::material::Material *mat, const std::string &textureID, Vulkan::Texture &hTex, const std::string &name);
			DLLCLIENT void GetTexture(lua::State *l, pragma::material::Material *mat, const std::string &textureID);
			DLLCLIENT void InitializeShaderData(lua::State *l, pragma::material::Material *mat, bool reload);
			DLLCLIENT void InitializeShaderData(lua::State *l, pragma::material::Material *mat);
		};
	};
	namespace TextureInfo {
		DLLCLIENT std::shared_ptr<pragma::material::Texture> GetTexture(lua::State *l, ::TextureInfo *tex);
		DLLCLIENT std::pair<uint32_t, uint32_t> GetSize(lua::State *l, ::TextureInfo *tex);
		DLLCLIENT uint32_t GetWidth(lua::State *l, ::TextureInfo *tex);
		DLLCLIENT uint32_t GetHeight(lua::State *l, ::TextureInfo *tex);
	};
};

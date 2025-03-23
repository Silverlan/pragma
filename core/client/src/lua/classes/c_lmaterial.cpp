/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/classes/c_lmaterial.h"
#include "luasystem.h"
#include "cmaterialmanager.h"
#include "textureinfo.h"
#include "pragma/rendering/shaders/world/c_shader_textured_base.hpp"
#include <cmaterial.h>
#include <cmaterial_manager2.hpp>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void Lua::Material::Client::SetTexture(lua_State *, ::Material *mat, const std::string &textureID, const std::string &tex)
{
	auto *cmat = static_cast<CMaterial *>(mat);
	cmat->SetTexture(textureID, tex);
	cmat->UpdateTextures();
	c_game->ReloadMaterialShader(static_cast<CMaterial *>(mat));
}
void Lua::Material::Client::SetTexture(lua_State *l, ::Material *mat, const std::string &textureID, ::Texture &tex)
{
	auto *cmat = static_cast<CMaterial *>(mat);
	cmat->SetTexture(textureID, &tex);
	cmat->UpdateTextures();
	c_game->ReloadMaterialShader(static_cast<CMaterial *>(mat));
}
void Lua::Material::Client::SetTexture(lua_State *l, ::Material *mat, const std::string &textureID, Lua::Vulkan::Texture &hTex, const std::string &name)
{
	auto *cmat = static_cast<CMaterial *>(mat);
	cmat->SetTexture(textureID, hTex);
	auto *texInfo = cmat->GetTextureInfo(textureID);
	if(texInfo) {
		texInfo->name = name;
		if(texInfo->texture)
			static_cast<Texture *>(texInfo->texture.get())->SetName(name);
	}
	cmat->UpdateTextures();
	c_game->ReloadMaterialShader(static_cast<CMaterial *>(mat));
}
void Lua::Material::Client::SetTexture(lua_State *l, ::Material *mat, const std::string &textureID, Lua::Vulkan::Texture &hTex) { SetTexture(l, mat, textureID, hTex, ""); }

void Lua::Material::Client::GetTexture(lua_State *l, ::Material *mat, const std::string &textureID)
{
	auto *tex = mat->GetTextureInfo(textureID);
	if(tex == nullptr)
		return;
	Lua::Push<::TextureInfo *>(l, tex);
}

void Lua::Material::Client::InitializeShaderData(lua_State *l, ::Material *mat, bool reload)
{
	auto shaderHandler = static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetShaderHandler();
	if(shaderHandler)
		shaderHandler(mat);
	auto *shader = static_cast<::pragma::ShaderTexturedBase *>(mat->GetUserData());
	if(shader == nullptr)
		return;
	shader->InitializeMaterialDescriptorSet(static_cast<CMaterial &>(*mat), reload);
}

void Lua::Material::Client::InitializeShaderData(lua_State *l, ::Material *mat) { InitializeShaderData(l, mat, false); }

///////////////////

std::shared_ptr<Texture> Lua::TextureInfo::GetTexture(lua_State *l, ::TextureInfo *tex)
{
	if(tex->texture == nullptr)
		return nullptr;
	return std::static_pointer_cast<Texture>(tex->texture);
}
std::pair<uint32_t, uint32_t> Lua::TextureInfo::GetSize(lua_State *l, ::TextureInfo *tex) { return {tex->width, tex->height}; }
uint32_t Lua::TextureInfo::GetWidth(lua_State *l, ::TextureInfo *tex) { return tex->width; }
uint32_t Lua::TextureInfo::GetHeight(lua_State *l, ::TextureInfo *tex) { return tex->height; }

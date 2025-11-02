// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_client.h"

module pragma.client;


import :scripting.lua.classes.material;
import :client_state;
import :game;


void Lua::Material::Client::SetTexture(lua_State *, msys::Material *mat, const std::string &textureID, const std::string &tex)
{
	auto *cmat = static_cast<msys::CMaterial *>(mat);
	cmat->SetTexture(textureID, tex);
	cmat->UpdateTextures();
	pragma::get_cgame()->ReloadMaterialShader(static_cast<msys::CMaterial *>(mat));
}
void Lua::Material::Client::SetTexture(lua_State *l, msys::Material *mat, const std::string &textureID, msys::Texture &tex)
{
	auto *cmat = static_cast<msys::CMaterial *>(mat);
	cmat->SetTexture(textureID, &tex);
	cmat->UpdateTextures();
	pragma::get_cgame()->ReloadMaterialShader(static_cast<msys::CMaterial *>(mat));
}
void Lua::Material::Client::SetTexture(lua_State *l, msys::Material *mat, const std::string &textureID, Lua::Vulkan::Texture &hTex, const std::string &name)
{
	auto *cmat = static_cast<msys::CMaterial *>(mat);
	cmat->SetTexture(textureID, hTex);
	auto *texInfo = cmat->GetTextureInfo(textureID);
	if(texInfo) {
		texInfo->name = name;
		if(texInfo->texture)
			static_cast<msys::Texture *>(texInfo->texture.get())->SetName(name);
	}
	cmat->UpdateTextures();
	pragma::get_cgame()->ReloadMaterialShader(static_cast<msys::CMaterial *>(mat));
}
void Lua::Material::Client::SetTexture(lua_State *l, msys::Material *mat, const std::string &textureID, Lua::Vulkan::Texture &hTex) { SetTexture(l, mat, textureID, hTex, ""); }

void Lua::Material::Client::GetTexture(lua_State *l, msys::Material *mat, const std::string &textureID)
{
	auto *tex = mat->GetTextureInfo(textureID);
	if(tex == nullptr)
		return;
	Lua::Push<::TextureInfo *>(l, tex);
}

void Lua::Material::Client::InitializeShaderData(lua_State *l, msys::Material *mat, bool reload)
{
	auto shaderHandler = static_cast<msys::CMaterialManager &>(pragma::get_client_state()->GetMaterialManager()).GetShaderHandler();
	if(shaderHandler)
		shaderHandler(mat);
	auto *shader = static_cast<::pragma::ShaderTexturedBase *>(mat->GetUserData());
	if(shader == nullptr)
		return;
	shader->InitializeMaterialDescriptorSet(static_cast<msys::CMaterial &>(*mat), reload);
}

void Lua::Material::Client::InitializeShaderData(lua_State *l, msys::Material *mat) { InitializeShaderData(l, mat, false); }

///////////////////

std::shared_ptr<msys::Texture> Lua::TextureInfo::GetTexture(lua_State *l, ::TextureInfo *tex)
{
	if(tex->texture == nullptr)
		return nullptr;
	return std::static_pointer_cast<msys::Texture>(tex->texture);
}
std::pair<uint32_t, uint32_t> Lua::TextureInfo::GetSize(lua_State *l, ::TextureInfo *tex) { return {tex->width, tex->height}; }
uint32_t Lua::TextureInfo::GetWidth(lua_State *l, ::TextureInfo *tex) { return tex->width; }
uint32_t Lua::TextureInfo::GetHeight(lua_State *l, ::TextureInfo *tex) { return tex->height; }

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lmaterial.h"
#include "luasystem.h"
#include "cmaterialmanager.h"
#include "textureinfo.h"
#include <cmaterial.h>

extern DLLCLIENT CGame *c_game;

void Lua::Material::Client::SetTexture(lua_State*,::Material *mat,const std::string &textureID,const std::string &tex)
{
	auto *cmat = static_cast<CMaterial*>(mat);
	cmat->SetTexture(textureID,tex);
	cmat->UpdateTextures();
	c_game->ReloadMaterialShader(static_cast<CMaterial*>(mat));
}
void Lua::Material::Client::SetTexture(lua_State *l,::Material *mat,const std::string &textureID,::Texture &tex)
{
	auto *cmat = static_cast<CMaterial*>(mat);
	cmat->SetTexture(textureID,&tex);
	cmat->UpdateTextures();
	c_game->ReloadMaterialShader(static_cast<CMaterial*>(mat));
}
void Lua::Material::Client::SetTexture(lua_State *l,::Material *mat,const std::string &textureID,Lua::Vulkan::Texture &hTex)
{
	auto *cmat = static_cast<CMaterial*>(mat);
	cmat->SetTexture(textureID,hTex);
	cmat->UpdateTextures();
	c_game->ReloadMaterialShader(static_cast<CMaterial*>(mat));
}

void Lua::Material::Client::GetTexture(lua_State *l,::Material *mat,const std::string &textureID)
{
	auto *tex = mat->GetTextureInfo(textureID);
	if(tex == nullptr)
		return;
	Lua::Push<::TextureInfo*>(l,tex);
}

void Lua::Material::Client::GetData(lua_State *l,::Material *mat)
{
	auto &data = mat->GetDataBlock();
	Lua::Push<std::shared_ptr<ds::Block>>(l,data);
}

///////////////////

void Lua::TextureInfo::GetTexture(lua_State *l,::TextureInfo *tex)
{
	if(tex->texture == nullptr)
		return;
	Lua::Push<Texture*>(l,static_cast<Texture*>(tex->texture.get()));
}
void Lua::TextureInfo::GetSize(lua_State *l,::TextureInfo *tex)
{
	Lua::PushInt(l,tex->width);
	Lua::PushInt(l,tex->height);
}
void Lua::TextureInfo::GetWidth(lua_State *l,::TextureInfo *tex)
{
	Lua::PushInt(l,tex->width);
}
void Lua::TextureInfo::GetHeight(lua_State *l,::TextureInfo *tex)
{
	Lua::PushInt(l,tex->height);
}

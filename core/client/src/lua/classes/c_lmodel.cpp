#include "stdafx_client.h"
#include "pragma/lua/classes/c_lmodel.h"
#include "luasystem.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/asset/c_util_model.hpp"
#include <pragma/lua/classes/lmodel.h>
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <pragma/lua/libraries/lfile.h>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void Lua::Model::Client::Export(lua_State *l,::Model &mdl,const pragma::asset::ModelExportInfo &exportInfo)
{
	std::string errMsg;
	auto result = pragma::asset::export_model(mdl,exportInfo,errMsg);
	Lua::PushBool(l,result);
	if(result == false)
		Lua::PushString(l,errMsg);
}
void Lua::Model::Client::ExportAnimation(lua_State *l,::Model &mdl,const std::string &animName,const pragma::asset::ModelExportInfo &exportInfo)
{
	std::string errMsg;
	auto result = pragma::asset::export_animation(mdl,animName,exportInfo,errMsg);
	Lua::PushBool(l,result);
	if(result == false)
		Lua::PushString(l,errMsg);
}
void Lua::Model::Client::AddMaterial(lua_State *l,::Model &mdl,uint32_t textureGroup,const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = client->LoadMaterial(name);
	Lua::Model::AddMaterial(l,mdl,textureGroup,mat);
}
void Lua::Model::Client::SetMaterial(lua_State *l,::Model &mdl,uint32_t texIdx,const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = client->LoadMaterial(name);
	Lua::Model::SetMaterial(l,mdl,texIdx,mat);
}
void Lua::Model::Client::GetVertexAnimationBuffer(lua_State *l,::Model &mdl)
{
	auto &buf = static_cast<CModel&>(mdl).GetVertexAnimationBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l,buf);
}

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lmodel.h"
#include "luasystem.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/classes/lmodel.h>
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"

extern DLLCLIENT ClientState *client;

void Lua::Model::Client::AddMaterial(lua_State *l,const std::shared_ptr<::Model> &mdl,uint32_t textureGroup,const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = client->LoadMaterial(name);
	Lua::Model::AddMaterial(l,mdl,textureGroup,mat);
}
void Lua::Model::Client::GetVertexAnimationBuffer(lua_State *l,const std::shared_ptr<::Model> &mdl)
{
	auto &buf = static_cast<CModel&>(*mdl).GetVertexAnimationBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l,buf);
}

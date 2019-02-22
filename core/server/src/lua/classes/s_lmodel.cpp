#include "stdafx_server.h"
#include "luasystem.h"
#include "pragma/lua/classes/s_lmodel.h"
#include <pragma/model/model.h>
#include "pragma/lua/classes/ldef_model.h"
#include <pragma/lua/classes/lmodel.h>

extern DLLSERVER ServerState *server;

void Lua::Model::Server::AddMaterial(lua_State *l,const std::shared_ptr<::Model> &mdl,uint32_t textureGroup,const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = server->LoadMaterial(name);
	Lua::Model::AddMaterial(l,mdl,textureGroup,mat);
}
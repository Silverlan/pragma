#include "stdafx_server.h"
#include "luasystem.h"
#include "pragma/lua/classes/s_lmodelmesh.h"
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/lua/classes/lmodelmesh.h>

void Lua::ModelMesh::Server::Create(lua_State *l)
{
	Lua::Push<std::shared_ptr<::ModelMesh>>(l,std::make_shared<::ModelMesh>());
}

void Lua::ModelSubMesh::Server::Create(lua_State *l)
{
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,std::make_shared<::ModelSubMesh>());
}

void Lua::ModelSubMesh::Server::CreateBox(lua_State *l,const Vector3 &min,const Vector3 &max)
{
	Lua::ModelSubMesh::CreateBox<::ModelSubMesh>(l,min,max);
}
void Lua::ModelSubMesh::Server::CreateSphere(lua_State *l,const Vector3 &origin,float radius,uint32_t recursionLevel)
{
	Lua::ModelSubMesh::CreateSphere<::ModelSubMesh>(l,origin,radius,recursionLevel);
}
void Lua::ModelSubMesh::Server::CreateSphere(lua_State *l,const Vector3 &origin,float radius)
{
	Lua::ModelSubMesh::CreateSphere<::ModelSubMesh>(l,origin,radius);
}

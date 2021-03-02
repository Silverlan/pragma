/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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

void Lua::ModelSubMesh::Server::CreateQuad(lua_State *l,float size)
{
	auto subMesh = std::make_shared<::ModelSubMesh>();
	Lua::ModelSubMesh::InitializeQuad(l,*subMesh,size);
}
void Lua::ModelSubMesh::Server::CreateBox(lua_State *l,const Vector3 &min,const Vector3 &max)
{
	auto subMesh = std::make_shared<::ModelSubMesh>();
	Lua::ModelSubMesh::InitializeBox(l,*subMesh,min,max);
}
void Lua::ModelSubMesh::Server::CreateSphere(lua_State *l,const Vector3 &origin,float radius,uint32_t recursionLevel)
{
	auto subMesh = std::make_shared<::ModelSubMesh>();
	Lua::ModelSubMesh::InitializeSphere(l,*subMesh,origin,radius,recursionLevel);
}
void Lua::ModelSubMesh::Server::CreateSphere(lua_State *l,const Vector3 &origin,float radius)
{
	auto subMesh = std::make_shared<::ModelSubMesh>();
	Lua::ModelSubMesh::InitializeSphere(l,*subMesh,origin,radius);
}
void Lua::ModelSubMesh::Server::CreateCylinder(lua_State *l,float startRadius,float length,uint32_t segmentCount)
{
	auto subMesh = std::make_shared<::ModelSubMesh>();
	Lua::ModelSubMesh::InitializeCylinder(l,*subMesh,startRadius,length,segmentCount);
}
void Lua::ModelSubMesh::Server::CreateCone(lua_State *l,float startRadius,float length,float endRadius,uint32_t segmentCount)
{
	auto subMesh = std::make_shared<::ModelSubMesh>();
	Lua::ModelSubMesh::InitializeCone(l,*subMesh,startRadius,length,endRadius,segmentCount);
}
void Lua::ModelSubMesh::Server::CreateCircle(lua_State *l,float radius,bool doubleSided,uint32_t segmentCount)
{
	auto subMesh = std::make_shared<::ModelSubMesh>();
	Lua::ModelSubMesh::InitializeCircle(l,*subMesh,radius,doubleSided,segmentCount);
}
void Lua::ModelSubMesh::Server::CreateRing(lua_State *l,float innerRadius,float outerRadius,bool doubleSided,uint32_t segmentCount)
{
	auto subMesh = std::make_shared<::ModelSubMesh>();
	Lua::ModelSubMesh::InitializeRing(l,*subMesh,innerRadius,outerRadius,doubleSided,segmentCount);
}

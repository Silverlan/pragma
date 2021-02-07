/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lcollisionmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/physics/shape.hpp"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include <pragma/lua/luaapi.h>

extern DLLNETWORK Engine *engine;

void Lua::CollisionMesh::register_class(luabind::class_<::CollisionMesh> &classDef)
{
	classDef.scope[luabind::def("Create",static_cast<void(*)(lua_State*)>(&Create))];
	classDef.scope[luabind::def("CreateBox",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&)>(&CreateBox))];
	classDef.def("GetVertices",&GetVertices);
	classDef.def("GetAABB",&GetAABB);
	classDef.def("GetBoneParentId",&GetBoneParentId);
	classDef.def("GetOrigin",&GetOrigin);
	classDef.def("GetShape",&GetShape);
	classDef.def("GetSurfaceMaterialId",&GetSurfaceMaterialId);
	classDef.def("GetSurfaceMaterialIds",&GetSurfaceMaterialIds);
	classDef.def("IntersectAABB",&IntersectAABB);
	classDef.def("IsConvex",&IsConvex);
	classDef.def("SetAABB",&SetAABB);
	classDef.def("SetBoneParentId",&SetBoneParentId);
	classDef.def("SetConvex",&SetConvex);
	classDef.def("SetOrigin",&SetOrigin);
	classDef.def("SetSurfaceMaterialId",&SetSurfaceMaterialId);
	classDef.def("Update",&Update);
	classDef.def("AddVertex",&AddVertex);
	classDef.def("GetVertexCount",&GetVertexCount);
	classDef.def("Rotate",&Rotate);
	classDef.def("Translate",&Translate);
	classDef.def("GetTriangles",&GetTriangles);
	classDef.def("ClipAgainstPlane",&ClipAgainstPlane);
	classDef.def("Centralize",&Centralize);
	classDef.def("GetVolume",&GetVolume);
	classDef.def("SetVolume",&SetVolume);
	classDef.def("ClearVertices",static_cast<void(*)(lua_State*,::CollisionMesh&)>([](lua_State *l,::CollisionMesh &mesh) {
		mesh.GetVertices().clear();
	}));
	classDef.def("ClearTriangles",static_cast<void(*)(lua_State*,::CollisionMesh&)>([](lua_State *l,::CollisionMesh &mesh) {
		mesh.GetTriangles().clear();
	}));
	classDef.def("AddTriangle",static_cast<void(*)(lua_State*,::CollisionMesh&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,::CollisionMesh &mesh,uint32_t idx0,uint32_t idx1,uint32_t idx2) {
		auto &triangles = mesh.GetTriangles();
		triangles.reserve(triangles.size() +3);
		triangles.push_back(idx0);
		triangles.push_back(idx1);
		triangles.push_back(idx2);
	}));
	classDef.def("SetVertices",static_cast<void(*)(lua_State*,::CollisionMesh&,luabind::object)>([](lua_State *l,::CollisionMesh &mesh,luabind::object tVertices) {
		auto idxVertices = 2;
		Lua::CheckTable(l,idxVertices);
		auto numVerts = Lua::GetObjectLength(l,idxVertices);
		auto &verts = mesh.GetVertices();
		verts.clear();
		verts.reserve(numVerts);
		for(auto i=decltype(numVerts){0u};i<numVerts;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,idxVertices);
			auto &v = Lua::Check<Vector3>(l,-1);
			verts.push_back(v);
			Lua::Pop(l,1);
		}
	}));
	classDef.def("SetTriangles",static_cast<void(*)(lua_State*,::CollisionMesh&,luabind::object)>([](lua_State *l,::CollisionMesh &mesh,luabind::object tTriangles) {
		auto idxTriangles = 2;
		Lua::CheckTable(l,idxTriangles);
		auto numTris = Lua::GetObjectLength(l,idxTriangles);
		auto &tris = mesh.GetTriangles();
		tris.clear();
		tris.reserve(numTris);
		for(auto i=decltype(numTris){0u};i<numTris;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,idxTriangles);
			auto idx = Lua::CheckInt(l,-1);
			tris.push_back(idx);
			Lua::Pop(l,1);
		}
	}));
	classDef.def("Copy",static_cast<void(*)(lua_State*,::CollisionMesh&)>([](lua_State *l,::CollisionMesh &mesh) {
		auto cpy = std::make_shared<::CollisionMesh>(mesh);
		Lua::Push<std::shared_ptr<::CollisionMesh>>(l,cpy);
	}));

	classDef.def("SetSoftBody",&SetSoftBody);
	classDef.def("IsSoftBody",&IsSoftBody);
	classDef.def("GetSoftBodyMesh",&GetSoftBodyMesh);
	classDef.def("SetSoftBodyMesh",&SetSoftBodyMesh);
	classDef.def("GetSoftBodyInfo",&GetSoftBodyInfo);
	classDef.def("GetSoftBodyTriangles",&GetSoftBodyTriangles);
	classDef.def("SetSoftBodyTriangles",&SetSoftBodyTriangles);

	classDef.def("AddSoftBodyAnchor",static_cast<void(*)(lua_State*,::CollisionMesh&,uint32_t,uint32_t,uint32_t,float)>(&AddSoftBodyAnchor));
	classDef.def("AddSoftBodyAnchor",static_cast<void(*)(lua_State*,::CollisionMesh&,uint32_t,uint32_t,uint32_t)>(&AddSoftBodyAnchor));
	classDef.def("AddSoftBodyAnchor",static_cast<void(*)(lua_State*,::CollisionMesh&,uint32_t,uint32_t)>(&AddSoftBodyAnchor));
	classDef.def("RemoveSoftBodyAnchor",&RemoveSoftBodyAnchor);
	classDef.def("ClearSoftBodyAnchors",&ClearSoftBodyAnchors);
	classDef.def("GetSoftBodyAnchors",&GetSoftBodyAnchors);

	classDef.add_static_constant("FSOFTBODY_ANCHOR_NONE",umath::to_integral(::CollisionMesh::SoftBodyAnchor::Flags::None));
	classDef.add_static_constant("FSOFTBODY_ANCHOR_RIGID",umath::to_integral(::CollisionMesh::SoftBodyAnchor::Flags::Rigid));
	classDef.add_static_constant("FSOFTBODY_ANCHOR_DISABLE_COLLISIONS",umath::to_integral(::CollisionMesh::SoftBodyAnchor::Flags::DisableCollisions));
}
void Lua::CollisionMesh::Create(lua_State *l)
{
	Lua::Push<std::shared_ptr<::CollisionMesh>>(l,::CollisionMesh::Create(engine->GetNetworkState(l)->GetGameState()));
}
void Lua::CollisionMesh::CreateBox(lua_State *l,const Vector3 &cmin,const Vector3 &cmax)
{
	auto min = cmin;
	auto max = cmax;
	uvec::to_min_max(min,max);
	auto mesh = ::CollisionMesh::Create(engine->GetNetworkState(l)->GetGameState());
	mesh->AddVertex(min);
	mesh->AddVertex(Vector3(max.x,min.y,min.z));
	mesh->AddVertex(Vector3(max.x,min.y,max.z));
	mesh->AddVertex(Vector3(max.x,max.y,min.z));
	mesh->AddVertex(max);
	mesh->AddVertex(Vector3(min.x,max.y,min.z));
	mesh->AddVertex(Vector3(min.x,min.y,max.z));
	mesh->AddVertex(Vector3(min.x,max.y,max.z));
	mesh->Update();
	Lua::Push<decltype(mesh)>(l,mesh);
}
void Lua::CollisionMesh::GetVertices(lua_State *l,::CollisionMesh &mesh)
{
	lua_newtable(l);
	int top = lua_gettop(l);
	std::vector<Vector3> &verts = mesh.GetVertices();
	for(int i=0;i<verts.size();i++)
	{
		luabind::object(l,verts[i]).push(l);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::CollisionMesh::GetVertexCount(lua_State *l,::CollisionMesh &mesh)
{
	Lua::PushInt(l,mesh.GetVertices().size());
}
void Lua::CollisionMesh::Rotate(lua_State*,::CollisionMesh &mesh,const Quat &rot)
{
	mesh.Rotate(rot);
}
void Lua::CollisionMesh::Translate(lua_State*,::CollisionMesh &mesh,const Vector3 &t)
{
	mesh.Translate(t);
}
void Lua::CollisionMesh::GetAABB(lua_State *l,::CollisionMesh &mesh)
{
	Vector3 min,max;
	mesh.GetAABB(&min,&max);
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
}
void Lua::CollisionMesh::GetBoneParentId(lua_State *l,::CollisionMesh &mesh)
{
	auto boneId = mesh.GetBoneParent();
	Lua::PushInt(l,boneId);
}
void Lua::CollisionMesh::GetOrigin(lua_State *l,::CollisionMesh &mesh)
{
	auto &origin = mesh.GetOrigin();
	Lua::Push<Vector3>(l,origin);
}
void Lua::CollisionMesh::GetShape(lua_State *l,::CollisionMesh &mesh)
{
	auto shape = mesh.GetShape();
	if(shape == nullptr)
		return;
	shape->Push(l);
}
void Lua::CollisionMesh::GetSurfaceMaterialId(lua_State *l,::CollisionMesh &mesh)
{
	auto surfaceMaterial = mesh.GetSurfaceMaterial();
	Lua::PushInt(l,surfaceMaterial);
}
void Lua::CollisionMesh::GetSurfaceMaterialIds(lua_State *l,::CollisionMesh &mesh)
{
	auto t = Lua::CreateTable(l);
	auto &surfaceMaterials = mesh.GetSurfaceMaterials();
	int32_t n = 1;
	for(auto matId : surfaceMaterials)
	{
		Lua::PushInt(l,n);
		Lua::PushInt(l,matId);
		Lua::SetTableValue(l,t);
		++n;
	}
}
void Lua::CollisionMesh::IntersectAABB(lua_State *l,::CollisionMesh &mesh,const Vector3 &min,const Vector3 &max)
{
	auto r = mesh.IntersectAABB(const_cast<Vector3*>(&min),const_cast<Vector3*>(&max));
	Lua::PushBool(l,r);
}
void Lua::CollisionMesh::IsConvex(lua_State *l,::CollisionMesh &mesh)
{
	Lua::PushBool(l,mesh.IsConvex());
}
void Lua::CollisionMesh::SetAABB(lua_State*,::CollisionMesh &mesh,const Vector3 &min,const Vector3 &max)
{
	mesh.SetAABB(const_cast<Vector3&>(min),const_cast<Vector3&>(max));
}
void Lua::CollisionMesh::SetBoneParentId(lua_State*,::CollisionMesh &mesh,int32_t boneId)
{
	mesh.SetBoneParent(boneId);
}
void Lua::CollisionMesh::SetConvex(lua_State*,::CollisionMesh &mesh,bool bConvex)
{
	mesh.SetConvex(bConvex);
}
void Lua::CollisionMesh::SetOrigin(lua_State*,::CollisionMesh &mesh,const Vector3 &origin)
{
	mesh.SetOrigin(origin);
}
void Lua::CollisionMesh::SetSurfaceMaterialId(lua_State*,::CollisionMesh &mesh,int32_t surfaceMaterialId)
{
	mesh.SetSurfaceMaterial(surfaceMaterialId);
}
void Lua::CollisionMesh::Update(lua_State*,::CollisionMesh &mesh)
{
	mesh.Update();
}
void Lua::CollisionMesh::AddVertex(lua_State*,::CollisionMesh &mesh,const Vector3 &v)
{
	mesh.AddVertex(v);
}
void Lua::CollisionMesh::GetTriangles(lua_State *l,::CollisionMesh &mesh)
{
	auto &triangles = mesh.GetTriangles();
	auto t = Lua::CreateTable(l);
	int32_t triangleIdx = 1;
	for(auto idx : triangles)
	{
		Lua::PushInt(l,triangleIdx++);
		Lua::PushInt(l,idx);
		Lua::SetTableValue(l,t);
	}
}
void Lua::CollisionMesh::ClipAgainstPlane(lua_State *l,::CollisionMesh &mdl,const Vector3 &n,double d,::CollisionMesh &clippedMeshA,::CollisionMesh &clippedMeshB)
{
	mdl.ClipAgainstPlane(n,d,clippedMeshA,clippedMeshB);
}
void Lua::CollisionMesh::Centralize(lua_State *l,::CollisionMesh &mesh) {mesh.Centralize();}
void Lua::CollisionMesh::GetVolume(lua_State *l,::CollisionMesh &mesh) {Lua::PushNumber(l,mesh.GetVolume());}
void Lua::CollisionMesh::SetVolume(lua_State *l,::CollisionMesh &mesh,float volume) {mesh.SetVolume(volume);}
void Lua::CollisionMesh::SetSoftBody(lua_State *l,::CollisionMesh &mesh,bool bSoftBody) {mesh.SetSoftBody(bSoftBody);}
void Lua::CollisionMesh::IsSoftBody(lua_State *l,::CollisionMesh &mesh) {Lua::PushBool(l,mesh.IsSoftBody());}
void Lua::CollisionMesh::GetSoftBodyMesh(lua_State *l,::CollisionMesh &mesh)
{
	auto *subMesh = mesh.GetSoftBodyMesh();
	if(subMesh == nullptr)
		return;
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,subMesh->shared_from_this());
}
void Lua::CollisionMesh::SetSoftBodyMesh(lua_State *l,::CollisionMesh &mesh,const std::shared_ptr<::ModelSubMesh> &subMesh) {mesh.SetSoftBodyMesh(*subMesh);}
void Lua::CollisionMesh::GetSoftBodyInfo(lua_State *l,::CollisionMesh &mesh)
{
	auto *sbInfo = mesh.GetSoftBodyInfo();
	if(sbInfo == nullptr)
		return;
	Lua::Push<PhysSoftBodyInfo*>(l,sbInfo);
}
void Lua::CollisionMesh::GetSoftBodyTriangles(lua_State *l,::CollisionMesh &mesh)
{
	auto *sbTriangles = mesh.GetSoftBodyTriangles();
	if(sbTriangles == nullptr)
		return;
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto vertIdx : *sbTriangles)
	{
		Lua::PushInt(l,idx++);
		Lua::PushInt(l,vertIdx);
		Lua::SetTableValue(l,t);
	}
}
void Lua::CollisionMesh::SetSoftBodyTriangles(lua_State *l,::CollisionMesh &mesh,luabind::object o)
{
	auto *sbTriangles = mesh.GetSoftBodyTriangles();
	if(sbTriangles == nullptr)
		return;
	auto t = 2u;
	Lua::CheckTable(l,t);
	auto numIndices = Lua::GetObjectLength(l,t);
	sbTriangles->clear();
	sbTriangles->resize(numIndices);
	for(auto i=decltype(numIndices){0};i<numIndices;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,t);
		sbTriangles->at(i) = Lua::CheckInt(l,-1);
		Lua::Pop(l,1);
	}
}
void Lua::CollisionMesh::AddSoftBodyAnchor(lua_State *l,::CollisionMesh &mesh,uint32_t vertIdx,uint32_t boneIdx,uint32_t flags,float influence)
{
	auto idx = 0u;
	auto b = mesh.AddSoftBodyAnchor(vertIdx,boneIdx,static_cast<::CollisionMesh::SoftBodyAnchor::Flags>(flags),influence,&idx);
	if(b == false)
		return;
	Lua::PushInt(l,idx);
}
void Lua::CollisionMesh::AddSoftBodyAnchor(lua_State *l,::CollisionMesh &mesh,uint32_t vertIdx,uint32_t boneIdx,uint32_t flags) {AddSoftBodyAnchor(l,mesh,vertIdx,boneIdx,flags,1.f);}
void Lua::CollisionMesh::AddSoftBodyAnchor(lua_State *l,::CollisionMesh &mesh,uint32_t vertIdx,uint32_t boneIdx){AddSoftBodyAnchor(l,mesh,vertIdx,boneIdx,0u,1.f);}
void Lua::CollisionMesh::RemoveSoftBodyAnchor(lua_State *l,::CollisionMesh &mesh,uint32_t anchorIdx) {mesh.RemoveSoftBodyAnchor(anchorIdx);}
void Lua::CollisionMesh::ClearSoftBodyAnchors(lua_State *l,::CollisionMesh &mesh) {mesh.ClearSoftBodyAnchors();}
static void push_soft_body_anchor(lua_State *l,const ::CollisionMesh::SoftBodyAnchor &anchor)
{
	auto t = Lua::CreateTable(l);

	Lua::PushString(l,"vertexIndex");
	Lua::PushInt(l,anchor.vertexIndex);
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"boneId");
	Lua::PushInt(l,anchor.boneId);
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"influence");
	Lua::PushNumber(l,anchor.influence);
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"flags");
	Lua::PushInt(l,umath::to_integral(anchor.flags));
	Lua::SetTableValue(l,t);
}
void Lua::CollisionMesh::GetSoftBodyAnchors(lua_State *l,::CollisionMesh &mesh)
{
	auto *anchors = mesh.GetSoftBodyAnchors();
	if(anchors == nullptr)
		return;
	auto t = Lua::CreateTable(l);
	auto anchorIdx = 1u;
	for(auto &anchor : *anchors)
	{
		Lua::PushInt(l,anchorIdx++);
		push_soft_body_anchor(l,anchor);
		Lua::SetTableValue(l,t);
	}
}

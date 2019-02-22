#include "stdafx_shared.h"
#include "pragma/lua/classes/lmodelmesh.h"
#include "luasystem.h"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"

extern DLLENGINE Engine *engine;

void Lua::ModelMesh::register_class(luabind::class_<std::shared_ptr<::ModelMesh>> &classDef)
{
	classDef.def(luabind::const_self == std::shared_ptr<::ModelMesh>());
	classDef.def("GetVertexCount",&Lua::ModelMesh::GetVertexCount);
	classDef.def("GetTriangleVertexCount",&Lua::ModelMesh::GetTriangleVertexCount);
	classDef.def("GetTriangleCount",&Lua::ModelMesh::GetTriangleCount);
	classDef.def("GetSubMeshes",&Lua::ModelMesh::GetSubMeshes);
	classDef.def("AddSubMesh",&Lua::ModelMesh::AddSubMesh);
	classDef.def("Update",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelMesh>&)>(&Lua::ModelMesh::Update));
	classDef.def("Update",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelMesh>&,uint32_t)>(&Lua::ModelMesh::Update));
	classDef.def("GetBounds",&Lua::ModelMesh::GetBounds);
	classDef.def("SetCenter",&Lua::ModelMesh::SetCenter);
	classDef.def("GetCenter",&Lua::ModelMesh::GetCenter);
	classDef.def("Centralize",&Lua::ModelMesh::Centralize);
	classDef.def("Scale",&Lua::ModelMesh::Scale);
	classDef.def("ClearSubMeshes",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelMesh>&)>([](lua_State *l,std::shared_ptr<::ModelMesh> &mesh) {
		mesh->GetSubMeshes().clear();
	}));
	classDef.def("SetSubMeshes",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelMesh>&,luabind::object)>([](lua_State *l,std::shared_ptr<::ModelMesh> &mesh,luabind::object tSubMeshes) {
		auto idxSubMeshes = 2;
		Lua::CheckTable(l,idxSubMeshes);
		auto &subMeshes = mesh->GetSubMeshes();
		subMeshes = {};
		auto numSubMeshes = Lua::GetObjectLength(l,idxSubMeshes);
		subMeshes.reserve(numSubMeshes);
		for(auto i=decltype(numSubMeshes){0u};i<numSubMeshes;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,idxSubMeshes);
			auto &subMesh = Lua::Check<std::shared_ptr<::ModelSubMesh>>(l,-1);
			subMeshes.push_back(subMesh);
			Lua::Pop(l,1);
		}
	}));
}
void Lua::ModelMesh::GetVertexCount(lua_State *l,std::shared_ptr<::ModelMesh> &mesh)
{
	Lua::PushInt(l,mesh->GetVertexCount());
}
void Lua::ModelMesh::GetTriangleVertexCount(lua_State *l,std::shared_ptr<::ModelMesh> &mesh)
{
	Lua::PushInt(l,mesh->GetTriangleVertexCount());
}
void Lua::ModelMesh::GetTriangleCount(lua_State *l,std::shared_ptr<::ModelMesh> &mdl)
{
	Lua::PushInt(l,mdl->GetTriangleCount());
}
void Lua::ModelMesh::GetSubMeshes(lua_State *l,std::shared_ptr<::ModelMesh> &mdl)
{
	auto &subMeshes = mdl->GetSubMeshes();
	Lua::CreateTable(l);
	auto top = Lua::GetStackTop(l);
	UInt i = 0;
	for(auto it=subMeshes.begin();it!=subMeshes.end();++it)
	{
		auto &subMesh = *it;
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,subMesh);
		Lua::SetTableValue(l,top,CInt32(i +1));
		i++;
	}
}
void Lua::ModelMesh::AddSubMesh(lua_State*,std::shared_ptr<::ModelMesh> &mdl,std::shared_ptr<::ModelSubMesh> &mesh)
{
	mdl->AddSubMesh(mesh);
}
void Lua::ModelMesh::Update(lua_State*,std::shared_ptr<::ModelMesh> &mdl)
{
	mdl->Update();
}
void Lua::ModelMesh::Update(lua_State*,std::shared_ptr<::ModelMesh> &mdl,uint32_t flags)
{
	mdl->Update(static_cast<ModelUpdateFlags>(flags));
}
void Lua::ModelMesh::GetBounds(lua_State *l,std::shared_ptr<::ModelMesh> &mdl)
{
	Vector3 min,max;
	mdl->GetBounds(min,max);
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
}
void Lua::ModelMesh::SetCenter(lua_State*,std::shared_ptr<::ModelMesh> &mdl,const Vector3 &center)
{
	mdl->SetCenter(center);
}
void Lua::ModelMesh::GetCenter(lua_State *l,std::shared_ptr<::ModelMesh> &mdl)
{
	Lua::Push<Vector3>(l,mdl->GetCenter());
}
void Lua::ModelMesh::Centralize(lua_State*,std::shared_ptr<::ModelMesh> &mdl) {mdl->Centralize();}
void Lua::ModelMesh::Scale(lua_State *l,std::shared_ptr<::ModelMesh> &mdl,const Vector3 &scale) {mdl->Scale(scale);}

////////////////////////////////////////

void Lua::ModelSubMesh::register_class(luabind::class_<std::shared_ptr<::ModelSubMesh>> &classDef)
{
	classDef.def(luabind::const_self == std::shared_ptr<::ModelSubMesh>());
	classDef.def("GetMaterialIndex",&Lua::ModelSubMesh::GetTexture);
	classDef.def("GetVertexCount",&Lua::ModelSubMesh::GetVertexCount);
	classDef.def("GetTriangleVertexCount",&Lua::ModelSubMesh::GetTriangleVertexCount);
	classDef.def("GetTriangleCount",&Lua::ModelSubMesh::GetTriangleCount);
	classDef.def("GetVertices",&Lua::ModelSubMesh::GetVertices);
	classDef.def("GetTriangles",&Lua::ModelSubMesh::GetTriangles);
	classDef.def("GetUVs",&Lua::ModelSubMesh::GetUVMapping);
	classDef.def("GetNormals",&Lua::ModelSubMesh::GetNormalMapping);
	classDef.def("GetVertexWeights",&Lua::ModelSubMesh::GetVertexWeights);
	classDef.def("AddTriangle",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelSubMesh>&,const Vertex&,const Vertex&,const Vertex&)>(&Lua::ModelSubMesh::AddTriangle));
	classDef.def("AddTriangle",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelSubMesh>&,uint32_t,uint32_t,uint32_t)>(&Lua::ModelSubMesh::AddTriangle));
	classDef.def("SetMaterialIndex",&Lua::ModelSubMesh::SetTexture);
	classDef.def("Update",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelSubMesh>&,uint32_t)>(&Lua::ModelSubMesh::Update));
	classDef.def("Update",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelSubMesh>&)>(&Lua::ModelSubMesh::Update));
	classDef.def("AddVertex",&Lua::ModelSubMesh::AddVertex);
	classDef.def("GetBounds",&Lua::ModelSubMesh::GetBounds);
	classDef.def("GetCenter",&Lua::ModelSubMesh::GetCenter);
	classDef.def("SetVertex",&Lua::ModelSubMesh::SetVertex);
	classDef.def("SetVertexPosition",&Lua::ModelSubMesh::SetVertexPosition);
	classDef.def("SetVertexNormal",&Lua::ModelSubMesh::SetVertexNormal);
	classDef.def("SetVertexUV",&Lua::ModelSubMesh::SetVertexUV);
	classDef.def("SetVertexAlpha",&Lua::ModelSubMesh::SetVertexAlpha);
	classDef.def("SetVertexWeight",&Lua::ModelSubMesh::SetVertexWeight);
	classDef.def("GetVertex",&Lua::ModelSubMesh::GetVertex);
	classDef.def("GetVertexPosition",&Lua::ModelSubMesh::GetVertexPosition);
	classDef.def("GetVertexNormal",&Lua::ModelSubMesh::GetVertexNormal);
	classDef.def("GetVertexUV",&Lua::ModelSubMesh::GetVertexUV);
	classDef.def("GetVertexAlpha",&Lua::ModelSubMesh::GetVertexAlpha);
	classDef.def("GetVertexWeight",&Lua::ModelSubMesh::GetVertexWeight);
	classDef.def("Optimize",&Lua::ModelSubMesh::Optimize);
	classDef.def("GenerateNormals",&Lua::ModelSubMesh::GenerateNormals);
	classDef.def("NormalizeUVCoordinates",&Lua::ModelSubMesh::NormalizeUVCoordinates);
	classDef.def("ClipAgainstPlane",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelSubMesh>&,const Vector3&,double,bool,luabind::object)>(&Lua::ModelSubMesh::ClipAgainstPlane));
	classDef.def("ClipAgainstPlane",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelSubMesh>&,const Vector3&,double,bool)>(&Lua::ModelSubMesh::ClipAgainstPlane));
	classDef.def("ClipAgainstPlane",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelSubMesh>&,const Vector3&,double)>(&Lua::ModelSubMesh::ClipAgainstPlane));
	classDef.def("ApplyUVMapping",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelSubMesh>&,const std::shared_ptr<::Model>&,const Vector3&,const Vector3&,float,float,float,float)>(&Lua::ModelSubMesh::ApplyUVMapping));
	classDef.def("ApplyUVMapping",static_cast<void(*)(lua_State*,std::shared_ptr<::ModelSubMesh>&,const Vector3&,const Vector3&,uint32_t,uint32_t,float,float,float,float)>(&Lua::ModelSubMesh::ApplyUVMapping));
	classDef.def("Scale",&Lua::ModelSubMesh::Scale);
}
void Lua::ModelSubMesh::GetTexture(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh)
{
	Lua::PushInt(l,mesh->GetTexture());
}
void Lua::ModelSubMesh::GetVertexCount(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl)
{
	Lua::PushInt(l,mdl->GetVertexCount());
}
void Lua::ModelSubMesh::GetTriangleVertexCount(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl)
{
	Lua::PushInt(l,mdl->GetTriangleVertexCount());
}
void Lua::ModelSubMesh::GetTriangleCount(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl)
{
	Lua::PushInt(l,mdl->GetTriangleCount());
}
void Lua::ModelSubMesh::GetVertices(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh)
{
	auto &verts = mesh->GetVertices();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i=0;i<verts.size();i++)
	{
		Lua::Push<Vector3>(l,verts[i].position);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::ModelSubMesh::GetTriangles(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh)
{
	auto &triangles = mesh->GetTriangles();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i=0;i<triangles.size();i++)
	{
		Lua::PushInt(l,triangles[i]);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::ModelSubMesh::GetUVMapping(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh)
{
	auto &verts = mesh->GetVertices();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i=0;i<verts.size();i++)
	{
		Lua::Push<Vector2>(l,verts[i].uv);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::ModelSubMesh::GetNormalMapping(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh)
{
	auto &verts = mesh->GetVertices();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i=0;i<verts.size();i++)
	{
		Lua::Push<Vector3>(l,verts[i].normal);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::ModelSubMesh::GetVertexWeights(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh)
{
	auto &vertWeights = mesh->GetVertexWeights();
	auto t = Lua::CreateTable(l); /* 1 */
	for(auto i=decltype(vertWeights.size()){0};i<vertWeights.size();++i)
	{
		auto &w = vertWeights[i];
		Lua::PushInt(l,i +1); /* 2 */

		auto tWeight = Lua::CreateTable(l); /* 3 */

		Lua::PushInt(l,1); /* 4 */
		Lua::Push<Vector4i>(l,w.boneIds); /* 5 */
		Lua::SetTableValue(l,tWeight); /* 3 */

		Lua::PushInt(l,2); /* 4 */
		Lua::Push<Vector4>(l,w.weights); /* 5 */
		Lua::SetTableValue(l,tWeight); /* 3 */

		Lua::SetTableValue(l,t); /* 1 */
	}
}
void Lua::ModelSubMesh::GetCenter(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl)
{
	Lua::Push<Vector3>(l,mdl->GetCenter());
}
void Lua::ModelSubMesh::AddTriangle(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,const Vertex &v1,const Vertex &v2,const Vertex &v3)
{
	mdl->AddTriangle(v1,v2,v3);
}
void Lua::ModelSubMesh::AddTriangle(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t a,uint32_t b,uint32_t c)
{
	mdl->AddTriangle(a,b,c);
}
void Lua::ModelSubMesh::SetTexture(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t texture)
{
	mdl->SetTexture(texture);
}
void Lua::ModelSubMesh::Update(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl)
{
	mdl->Update();
}
void Lua::ModelSubMesh::Update(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t flags)
{
	mdl->Update(static_cast<ModelUpdateFlags>(flags));
}
void Lua::ModelSubMesh::AddVertex(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const Vertex &v)
{
	auto idx = mdl->AddVertex(v);
	Lua::PushInt(l,idx);
}
void Lua::ModelSubMesh::GetBounds(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl)
{
	Vector3 min,max;
	mdl->GetBounds(min,max);
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
}

void Lua::ModelSubMesh::SetVertex(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const Vertex &v)
{
	mdl->SetVertex(idx,v);
}
void Lua::ModelSubMesh::SetVertexPosition(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const Vector3 &pos)
{
	mdl->SetVertexPosition(idx,pos);
}
void Lua::ModelSubMesh::SetVertexNormal(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const Vector3 &normal)
{
	mdl->SetVertexNormal(idx,normal);
}
void Lua::ModelSubMesh::SetVertexUV(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const Vector2 &uv)
{
	mdl->SetVertexUV(idx,uv);
}
void Lua::ModelSubMesh::SetVertexAlpha(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const Vector2 &alpha)
{
	mdl->SetVertexAlpha(idx,alpha);
}
void Lua::ModelSubMesh::SetVertexWeight(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const VertexWeight &weight)
{
	mdl->SetVertexWeight(idx,weight);
}
void Lua::ModelSubMesh::GetVertex(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx)
{
	if(idx >= mdl->GetVertexCount())
		return;
	Lua::Push<Vertex>(l,mdl->GetVertex(idx));
}
void Lua::ModelSubMesh::GetVertexPosition(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx)
{
	if(idx >= mdl->GetVertexCount())
		return;
	Lua::Push<Vector3>(l,mdl->GetVertexPosition(idx));
}
void Lua::ModelSubMesh::GetVertexNormal(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx)
{
	if(idx >= mdl->GetVertexCount())
		return;
	Lua::Push<Vector3>(l,mdl->GetVertexNormal(idx));
}
void Lua::ModelSubMesh::GetVertexUV(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx)
{
	if(idx >= mdl->GetVertexCount())
		return;
	Lua::Push<Vector2>(l,mdl->GetVertexUV(idx));
}
void Lua::ModelSubMesh::GetVertexAlpha(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx)
{
	if(idx >= mdl->GetAlphaCount())
		return;
	Lua::Push<Vector2>(l,mdl->GetVertexAlpha(idx));
}
void Lua::ModelSubMesh::GetVertexWeight(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx)
{
	if(idx >= mdl->GetVertexWeights().size())
		return;
	Lua::Push<VertexWeight>(l,mdl->GetVertexWeight(idx));
}
void Lua::ModelSubMesh::Optimize(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl)
{
	mdl->Optimize();
}
void Lua::ModelSubMesh::GenerateNormals(lua_State*,std::shared_ptr<::ModelSubMesh> &mdl)
{
	mdl->GenerateNormals();
}
void Lua::ModelSubMesh::NormalizeUVCoordinates(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl) {mdl->NormalizeUVCoordinates();}
void Lua::ModelSubMesh::ClipAgainstPlane(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const Vector3 &n,double d) {ClipAgainstPlane(l,mdl,n,d,false);}
void Lua::ModelSubMesh::ClipAgainstPlane(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const Vector3 &n,double d,bool bSplitCoverMeshes)
{
	auto clippedMeshA = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	auto clippedMeshB = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	std::shared_ptr<::ModelSubMesh> clippedCoverA = nullptr;
	std::shared_ptr<::ModelSubMesh> clippedCoverB = nullptr;
	if(bSplitCoverMeshes)
	{
		clippedCoverA = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
		clippedCoverB = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	}
	mdl->ClipAgainstPlane(n,d,*clippedMeshA,*clippedMeshB,nullptr,clippedCoverA.get(),clippedCoverB.get());
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedMeshA);
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedMeshB);
	if(bSplitCoverMeshes)
	{
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedCoverA);
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedCoverB);
	}
}
void Lua::ModelSubMesh::ClipAgainstPlane(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const Vector3 &n,double d,bool bSplitCoverMeshes,luabind::object tBoneMatrices)
{
	const auto tMatrices = 5;
	Lua::CheckTable(l,tMatrices);
	std::vector<Mat4> boneMatrices {};
	auto numMatrices = Lua::GetObjectLength(l,tMatrices);
	for(auto i=decltype(numMatrices){0u};i<numMatrices;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,tMatrices);
		auto &m = Lua::Check<Mat4>(l,-1);
		boneMatrices.push_back(m);
		Lua::Pop(l,1);
	}
	auto clippedMeshA = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	auto clippedMeshB = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	std::shared_ptr<::ModelSubMesh> clippedCoverA = nullptr;
	std::shared_ptr<::ModelSubMesh> clippedCoverB = nullptr;
	if(bSplitCoverMeshes)
	{
		clippedCoverA = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
		clippedCoverB = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	}
	mdl->ClipAgainstPlane(n,d,*clippedMeshA,*clippedMeshB,&boneMatrices,clippedCoverA.get(),clippedCoverB.get());
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedMeshA);
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedMeshB);
	if(bSplitCoverMeshes)
	{
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedCoverA);
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedCoverB);
	}
}
void Lua::ModelSubMesh::ApplyUVMapping(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const Vector3 &nu,const Vector3 &nv,uint32_t w,uint32_t h,float ou,float ov,float su,float sv)
{
	mdl->ApplyUVMapping(nu,nv,w,h,ou,ov,su,sv);
}
void Lua::ModelSubMesh::ApplyUVMapping(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh,const std::shared_ptr<::Model> &mdl,const Vector3 &nu,const Vector3 &nv,float ou,float ov,float su,float sv)
{
	auto matId = mesh->GetTexture();
	auto *mat = mdl->GetMaterial(matId);
	auto w = 0u;
	auto h = 0u;
	if(mat != nullptr)
	{
		auto *texInfo = mat->GetDiffuseMap();
		if(texInfo != nullptr)
		{
			w = texInfo->width;
			h = texInfo->height;
		}
	}
	ApplyUVMapping(l,mesh,nu,nv,w,h,ou,ov,su,sv);
}
void Lua::ModelSubMesh::Scale(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh,const Vector3 &scale) {mesh->Scale(scale);}

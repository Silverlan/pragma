#include "stdafx_shared.h"
#include "pragma/lua/classes/lmodelmesh.h"
#include "luasystem.h"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"

extern DLLENGINE Engine *engine;


void Lua::ModelMesh::register_class(luabind::class_<::ModelMesh> &classDef)
{
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def("GetVertexCount",&Lua::ModelMesh::GetVertexCount);
	classDef.def("GetTriangleVertexCount",&Lua::ModelMesh::GetTriangleVertexCount);
	classDef.def("GetTriangleCount",&Lua::ModelMesh::GetTriangleCount);
	classDef.def("GetSubMeshes",&Lua::ModelMesh::GetSubMeshes);
	classDef.def("AddSubMesh",&Lua::ModelMesh::AddSubMesh);
	classDef.def("Update",static_cast<void(*)(lua_State*,::ModelMesh&)>(&Lua::ModelMesh::Update));
	classDef.def("Update",static_cast<void(*)(lua_State*,::ModelMesh&,uint32_t)>(&Lua::ModelMesh::Update));
	classDef.def("GetBounds",&Lua::ModelMesh::GetBounds);
	classDef.def("SetCenter",&Lua::ModelMesh::SetCenter);
	classDef.def("GetCenter",&Lua::ModelMesh::GetCenter);
	classDef.def("Centralize",&Lua::ModelMesh::Centralize);
	classDef.def("Scale",&Lua::ModelMesh::Scale);
	classDef.def("Translate",static_cast<void(*)(lua_State*,::ModelMesh&,const Vector3&)>([](lua_State *l,::ModelMesh &mesh,const Vector3 &translation) {
		mesh.Translate(translation);
	}));
	classDef.def("Rotate",static_cast<void(*)(lua_State*,::ModelMesh&,const Quat&)>([](lua_State *l,::ModelMesh &mesh,const Quat &rotation) {
		mesh.Rotate(rotation);
	}));
	classDef.def("ClearSubMeshes",static_cast<void(*)(lua_State*,::ModelMesh&)>([](lua_State *l,::ModelMesh &mesh) {
		mesh.GetSubMeshes().clear();
	}));
	classDef.def("SetSubMeshes",static_cast<void(*)(lua_State*,::ModelMesh&,luabind::object)>([](lua_State *l,::ModelMesh &mesh,luabind::object tSubMeshes) {
		auto idxSubMeshes = 2;
		Lua::CheckTable(l,idxSubMeshes);
		auto &subMeshes = mesh.GetSubMeshes();
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
	classDef.def("GetSubMeshCount",static_cast<void(*)(lua_State*,::ModelMesh&)>([](lua_State *l,::ModelMesh &mesh) {
		Lua::PushInt(l,mesh.GetSubMeshes().size());
	}));
	classDef.def("GetSubMesh",static_cast<void(*)(lua_State*,::ModelMesh&,uint32_t)>([](lua_State *l,::ModelMesh &mesh,uint32_t index) {
		auto &subMeshes = mesh.GetSubMeshes();
		if(index >= subMeshes.size())
			return;
		Lua::Push(l,subMeshes.at(index));
	}));
}
void Lua::ModelMesh::GetVertexCount(lua_State *l,::ModelMesh &mesh)
{
	Lua::PushInt(l,mesh.GetVertexCount());
}
void Lua::ModelMesh::GetTriangleVertexCount(lua_State *l,::ModelMesh &mesh)
{
	Lua::PushInt(l,mesh.GetTriangleVertexCount());
}
void Lua::ModelMesh::GetTriangleCount(lua_State *l,::ModelMesh &mdl)
{
	Lua::PushInt(l,mdl.GetTriangleCount());
}
void Lua::ModelMesh::GetSubMeshes(lua_State *l,::ModelMesh &mdl)
{
	auto &subMeshes = mdl.GetSubMeshes();
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
void Lua::ModelMesh::AddSubMesh(lua_State*,::ModelMesh &mdl,::ModelSubMesh &mesh)
{
	mdl.AddSubMesh(mesh.shared_from_this());
}
void Lua::ModelMesh::Update(lua_State*,::ModelMesh &mdl)
{
	mdl.Update();
}
void Lua::ModelMesh::Update(lua_State*,::ModelMesh &mdl,uint32_t flags)
{
	mdl.Update(static_cast<ModelUpdateFlags>(flags));
}
void Lua::ModelMesh::GetBounds(lua_State *l,::ModelMesh &mdl)
{
	Vector3 min,max;
	mdl.GetBounds(min,max);
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
}
void Lua::ModelMesh::SetCenter(lua_State*,::ModelMesh &mdl,const Vector3 &center)
{
	mdl.SetCenter(center);
}
void Lua::ModelMesh::GetCenter(lua_State *l,::ModelMesh &mdl)
{
	Lua::Push<Vector3>(l,mdl.GetCenter());
}
void Lua::ModelMesh::Centralize(lua_State*,::ModelMesh &mdl) {mdl.Centralize();}
void Lua::ModelMesh::Scale(lua_State *l,::ModelMesh &mdl,const Vector3 &scale) {mdl.Scale(scale);}

////////////////////////////////////////

void Lua::ModelSubMesh::register_class(luabind::class_<::ModelSubMesh> &classDef)
{
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def("GetSkinTextureIndex",&Lua::ModelSubMesh::GetSkinTextureIndex);
	classDef.def("GetVertexCount",&Lua::ModelSubMesh::GetVertexCount);
	classDef.def("GetTriangleVertexCount",&Lua::ModelSubMesh::GetTriangleVertexCount);
	classDef.def("GetTriangleCount",&Lua::ModelSubMesh::GetTriangleCount);
	classDef.def("GetVertices",&Lua::ModelSubMesh::GetVertices);
	classDef.def("GetTriangles",&Lua::ModelSubMesh::GetTriangles);
	classDef.def("GetUVs",&Lua::ModelSubMesh::GetUVMapping);
	classDef.def("GetNormals",&Lua::ModelSubMesh::GetNormalMapping);
	classDef.def("GetVertexWeights",&Lua::ModelSubMesh::GetVertexWeights);
	classDef.def("AddTriangle",static_cast<void(*)(lua_State*,::ModelSubMesh&,const Vertex&,const Vertex&,const Vertex&)>(&Lua::ModelSubMesh::AddTriangle));
	classDef.def("AddTriangle",static_cast<void(*)(lua_State*,::ModelSubMesh&,uint32_t,uint32_t,uint32_t)>(&Lua::ModelSubMesh::AddTriangle));
	classDef.def("SetSkinTextureIndex",&Lua::ModelSubMesh::SetSkinTextureIndex);
	classDef.def("Update",static_cast<void(*)(lua_State*,::ModelSubMesh&,uint32_t)>(&Lua::ModelSubMesh::Update));
	classDef.def("Update",static_cast<void(*)(lua_State*,::ModelSubMesh&)>(&Lua::ModelSubMesh::Update));
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
	classDef.def("ClipAgainstPlane",static_cast<void(*)(lua_State*,::ModelSubMesh&,const Vector3&,double,bool,luabind::object)>(&Lua::ModelSubMesh::ClipAgainstPlane));
	classDef.def("ClipAgainstPlane",static_cast<void(*)(lua_State*,::ModelSubMesh&,const Vector3&,double,bool)>(&Lua::ModelSubMesh::ClipAgainstPlane));
	classDef.def("ClipAgainstPlane",static_cast<void(*)(lua_State*,::ModelSubMesh&,const Vector3&,double)>(&Lua::ModelSubMesh::ClipAgainstPlane));
	classDef.def("ApplyUVMapping",static_cast<void(*)(lua_State*,::ModelSubMesh&,::Model&,const Vector3&,const Vector3&,float,float,float,float)>(&Lua::ModelSubMesh::ApplyUVMapping));
	classDef.def("ApplyUVMapping",static_cast<void(*)(lua_State*,::ModelSubMesh&,const Vector3&,const Vector3&,uint32_t,uint32_t,float,float,float,float)>(&Lua::ModelSubMesh::ApplyUVMapping));
	classDef.def("Scale",&Lua::ModelSubMesh::Scale);
	classDef.def("Translate",static_cast<void(*)(lua_State*,::ModelSubMesh&,const Vector3&)>([](lua_State *l,::ModelSubMesh &mesh,const Vector3 &translation) {
		mesh.Translate(translation);
	}));
	classDef.def("Rotate",static_cast<void(*)(lua_State*,::ModelSubMesh&,const Quat&)>([](lua_State *l,::ModelSubMesh &mesh,const Quat &rotation) {
		mesh.Rotate(rotation);
	}));
	classDef.def("GetReferenceId",static_cast<void(*)(lua_State*,::ModelSubMesh&)>([](lua_State *l,::ModelSubMesh &mesh) {
		Lua::PushInt(l,mesh.GetReferenceId());
	}));
	classDef.def("GetGeometryType",static_cast<void(*)(lua_State*,::ModelSubMesh&)>([](lua_State *l,::ModelSubMesh &mesh) {
		Lua::PushInt(l,umath::to_integral(mesh.GetGeometryType()));
	}));
	classDef.def("SetGeometryType",static_cast<void(*)(lua_State*,::ModelSubMesh&,uint32_t)>([](lua_State *l,::ModelSubMesh &mesh,uint32_t geometryType) {
		mesh.SetGeometryType(static_cast<::ModelSubMesh::GeometryType>(geometryType));
	}));
	classDef.def("AddLine",static_cast<void(*)(lua_State*,::ModelSubMesh&,uint32_t,uint32_t)>([](lua_State *l,::ModelSubMesh &mesh,uint32_t idx0,uint32_t idx1) {
		mesh.AddLine(idx0,idx1);
	}));
	classDef.def("AddPoint",static_cast<void(*)(lua_State*,::ModelSubMesh&,uint32_t)>([](lua_State *l,::ModelSubMesh &mesh,uint32_t idx) {
		mesh.AddPoint(idx);
	}));
	classDef.def("GetPose",static_cast<void(*)(lua_State*,::ModelSubMesh&)>([](lua_State *l,::ModelSubMesh &mesh) {
		Lua::Push<pragma::physics::ScaledTransform>(l,mesh.GetPose());
	}));
	classDef.def("SetPose",static_cast<void(*)(lua_State*,::ModelSubMesh&,const pragma::physics::ScaledTransform&)>([](lua_State *l,::ModelSubMesh &mesh,const pragma::physics::ScaledTransform &pose) {
		mesh.SetPose(pose);
	}));
	classDef.def("Transform",static_cast<void(*)(lua_State*,::ModelSubMesh&,const pragma::physics::ScaledTransform&)>([](lua_State *l,::ModelSubMesh &mesh,const pragma::physics::ScaledTransform &pose) {
		mesh.Transform(pose);
	}));
	classDef.add_static_constant("GEOMETRY_TYPE_TRIANGLES",umath::to_integral(::ModelSubMesh::GeometryType::Triangles));
	classDef.add_static_constant("GEOMETRY_TYPE_LINES",umath::to_integral(::ModelSubMesh::GeometryType::Lines));
	classDef.add_static_constant("GEOMETRY_TYPE_POINTS",umath::to_integral(::ModelSubMesh::GeometryType::Points));
}
void Lua::ModelSubMesh::GetSkinTextureIndex(lua_State *l,::ModelSubMesh &mesh)
{
	Lua::PushInt(l,mesh.GetSkinTextureIndex());
}
void Lua::ModelSubMesh::GetVertexCount(lua_State *l,::ModelSubMesh &mdl)
{
	Lua::PushInt(l,mdl.GetVertexCount());
}
void Lua::ModelSubMesh::GetTriangleVertexCount(lua_State *l,::ModelSubMesh &mdl)
{
	Lua::PushInt(l,mdl.GetTriangleVertexCount());
}
void Lua::ModelSubMesh::GetTriangleCount(lua_State *l,::ModelSubMesh &mdl)
{
	Lua::PushInt(l,mdl.GetTriangleCount());
}
void Lua::ModelSubMesh::GetVertices(lua_State *l,::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i=0;i<verts.size();i++)
	{
		Lua::Push<Vector3>(l,verts[i].position);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::ModelSubMesh::GetTriangles(lua_State *l,::ModelSubMesh &mesh)
{
	auto &triangles = mesh.GetTriangles();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i=0;i<triangles.size();i++)
	{
		Lua::PushInt(l,triangles[i]);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::ModelSubMesh::GetUVMapping(lua_State *l,::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i=0;i<verts.size();i++)
	{
		Lua::Push<Vector2>(l,verts[i].uv);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::ModelSubMesh::GetNormalMapping(lua_State *l,::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i=0;i<verts.size();i++)
	{
		Lua::Push<Vector3>(l,verts[i].normal);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::ModelSubMesh::GetVertexWeights(lua_State *l,::ModelSubMesh &mesh)
{
	auto &vertWeights = mesh.GetVertexWeights();
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
void Lua::ModelSubMesh::GetCenter(lua_State *l,::ModelSubMesh &mdl)
{
	Lua::Push<Vector3>(l,mdl.GetCenter());
}
void Lua::ModelSubMesh::AddTriangle(lua_State*,::ModelSubMesh &mdl,const Vertex &v1,const Vertex &v2,const Vertex &v3)
{
	mdl.AddTriangle(v1,v2,v3);
}
void Lua::ModelSubMesh::AddTriangle(lua_State*,::ModelSubMesh &mdl,uint32_t a,uint32_t b,uint32_t c)
{
	mdl.AddTriangle(a,b,c);
}
void Lua::ModelSubMesh::SetSkinTextureIndex(lua_State*,::ModelSubMesh &mdl,uint32_t texture)
{
	mdl.SetSkinTextureIndex(texture);
}
void Lua::ModelSubMesh::Update(lua_State*,::ModelSubMesh &mdl)
{
	mdl.Update();
}
void Lua::ModelSubMesh::Update(lua_State*,::ModelSubMesh &mdl,uint32_t flags)
{
	mdl.Update(static_cast<ModelUpdateFlags>(flags));
}
void Lua::ModelSubMesh::AddVertex(lua_State *l,::ModelSubMesh &mdl,const Vertex &v)
{
	auto idx = mdl.AddVertex(v);
	Lua::PushInt(l,idx);
}
void Lua::ModelSubMesh::GetBounds(lua_State *l,::ModelSubMesh &mdl)
{
	Vector3 min,max;
	mdl.GetBounds(min,max);
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
}

void Lua::ModelSubMesh::SetVertex(lua_State*,::ModelSubMesh &mdl,uint32_t idx,const Vertex &v)
{
	mdl.SetVertex(idx,v);
}
void Lua::ModelSubMesh::SetVertexPosition(lua_State*,::ModelSubMesh &mdl,uint32_t idx,const Vector3 &pos)
{
	mdl.SetVertexPosition(idx,pos);
}
void Lua::ModelSubMesh::SetVertexNormal(lua_State*,::ModelSubMesh &mdl,uint32_t idx,const Vector3 &normal)
{
	mdl.SetVertexNormal(idx,normal);
}
void Lua::ModelSubMesh::SetVertexUV(lua_State*,::ModelSubMesh &mdl,uint32_t idx,const Vector2 &uv)
{
	mdl.SetVertexUV(idx,uv);
}
void Lua::ModelSubMesh::SetVertexAlpha(lua_State*,::ModelSubMesh &mdl,uint32_t idx,const Vector2 &alpha)
{
	mdl.SetVertexAlpha(idx,alpha);
}
void Lua::ModelSubMesh::SetVertexWeight(lua_State*,::ModelSubMesh &mdl,uint32_t idx,const VertexWeight &weight)
{
	mdl.SetVertexWeight(idx,weight);
}
void Lua::ModelSubMesh::GetVertex(lua_State *l,::ModelSubMesh &mdl,uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<Vertex>(l,mdl.GetVertex(idx));
}
void Lua::ModelSubMesh::GetVertexPosition(lua_State *l,::ModelSubMesh &mdl,uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<Vector3>(l,mdl.GetVertexPosition(idx));
}
void Lua::ModelSubMesh::GetVertexNormal(lua_State *l,::ModelSubMesh &mdl,uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<Vector3>(l,mdl.GetVertexNormal(idx));
}
void Lua::ModelSubMesh::GetVertexUV(lua_State *l,::ModelSubMesh &mdl,uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<Vector2>(l,mdl.GetVertexUV(idx));
}
void Lua::ModelSubMesh::GetVertexAlpha(lua_State *l,::ModelSubMesh &mdl,uint32_t idx)
{
	if(idx >= mdl.GetAlphaCount())
		return;
	Lua::Push<Vector2>(l,mdl.GetVertexAlpha(idx));
}
void Lua::ModelSubMesh::GetVertexWeight(lua_State *l,::ModelSubMesh &mdl,uint32_t idx)
{
	if(idx >= mdl.GetVertexWeights().size())
		return;
	Lua::Push<VertexWeight>(l,mdl.GetVertexWeight(idx));
}
void Lua::ModelSubMesh::Optimize(lua_State*,::ModelSubMesh &mdl)
{
	mdl.Optimize();
}
void Lua::ModelSubMesh::GenerateNormals(lua_State*,::ModelSubMesh &mdl)
{
	mdl.GenerateNormals();
}
void Lua::ModelSubMesh::NormalizeUVCoordinates(lua_State *l,::ModelSubMesh &mdl) {mdl.NormalizeUVCoordinates();}
void Lua::ModelSubMesh::ClipAgainstPlane(lua_State *l,::ModelSubMesh &mdl,const Vector3 &n,double d) {ClipAgainstPlane(l,mdl,n,d,false);}
void Lua::ModelSubMesh::ClipAgainstPlane(lua_State *l,::ModelSubMesh &mdl,const Vector3 &n,double d,bool bSplitCoverMeshes)
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
	mdl.ClipAgainstPlane(n,d,*clippedMeshA,*clippedMeshB,nullptr,clippedCoverA.get(),clippedCoverB.get());
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedMeshA);
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedMeshB);
	if(bSplitCoverMeshes)
	{
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedCoverA);
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedCoverB);
	}
}
void Lua::ModelSubMesh::ClipAgainstPlane(lua_State *l,::ModelSubMesh &mdl,const Vector3 &n,double d,bool bSplitCoverMeshes,luabind::object tBoneMatrices)
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
	mdl.ClipAgainstPlane(n,d,*clippedMeshA,*clippedMeshB,&boneMatrices,clippedCoverA.get(),clippedCoverB.get());
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedMeshA);
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedMeshB);
	if(bSplitCoverMeshes)
	{
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedCoverA);
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,clippedCoverB);
	}
}
void Lua::ModelSubMesh::ApplyUVMapping(lua_State *l,::ModelSubMesh &mdl,const Vector3 &nu,const Vector3 &nv,uint32_t w,uint32_t h,float ou,float ov,float su,float sv)
{
	mdl.ApplyUVMapping(nu,nv,w,h,ou,ov,su,sv);
}
void Lua::ModelSubMesh::ApplyUVMapping(lua_State *l,::ModelSubMesh &mesh,::Model &mdl,const Vector3 &nu,const Vector3 &nv,float ou,float ov,float su,float sv)
{
	auto matId = mdl.GetMaterialIndex(mesh);
	auto *mat = mdl.GetMaterial(matId.has_value() ? *matId : 0);
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
void Lua::ModelSubMesh::Scale(lua_State *l,::ModelSubMesh &mesh,const Vector3 &scale) {mesh.Scale(scale);}

void Lua::ModelSubMesh::InitializeQuad(lua_State *l,::ModelSubMesh &mesh,float size)
{
	Vector3 min {-size,0.f,-size};
	Vector3 max {size,0.f,size};
	std::vector<Vector3> uniqueVertices {
		min, // 0
		Vector3{max.x,min.y,min.z}, // 1
		Vector3{max.x,min.y,max.z}, // 2
		Vector3{min.x,min.y,max.z} // 3
	};
	std::vector<Vector3> verts {
		uniqueVertices.at(0),uniqueVertices.at(2),uniqueVertices.at(1),
		uniqueVertices.at(2),uniqueVertices.at(0),uniqueVertices.at(3)
	};
	std::vector<Vector3> faceNormals {
		uvec::UP,uvec::UP
	};
	std::vector<::Vector2> uvs {
		::Vector2{0.f,0.f},::Vector2{1.f,1.f},::Vector2{1.f,0.f},
		::Vector2{1.f,1.f},::Vector2{0.f,0.f},::Vector2{0.f,1.f}
	};
	for(auto i=decltype(verts.size()){0};i<verts.size();i+=3)
	{
		auto &n = faceNormals[i /3];
		mesh.AddVertex(::Vertex{verts[i],uvs[i],n});
		mesh.AddVertex(::Vertex{verts[i +1],uvs[i +1],n});
		mesh.AddVertex(::Vertex{verts[i +2],uvs[i +2],n});

		mesh.AddTriangle(static_cast<uint32_t>(i),static_cast<uint32_t>(i +1),static_cast<uint32_t>(i +2));
	}
	mesh.SetSkinTextureIndex(0);
	mesh.Update();
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,mesh.shared_from_this());
}

void Lua::ModelSubMesh::InitializeBox(lua_State *l,::ModelSubMesh &mesh,const Vector3 &cmin,const Vector3 &cmax)
{
	auto min = cmin;
	auto max = cmax;
	uvec::to_min_max(min,max);
	std::vector<Vector3> uniqueVertices {
		min, // 0
		Vector3(max.x,min.y,min.z), // 1
		Vector3(max.x,min.y,max.z), // 2
		Vector3(max.x,max.y,min.z), // 3
		max, // 4
		Vector3(min.x,max.y,min.z), // 5
		Vector3(min.x,min.y,max.z), // 6
		Vector3(min.x,max.y,max.z) // 7
	};
	std::vector<Vector3> verts {
		uniqueVertices[0],uniqueVertices[6],uniqueVertices[7], // 1
		uniqueVertices[0],uniqueVertices[7],uniqueVertices[5], // 1
		uniqueVertices[3],uniqueVertices[0],uniqueVertices[5], // 2
		uniqueVertices[3],uniqueVertices[1],uniqueVertices[0], // 2
		uniqueVertices[2],uniqueVertices[0],uniqueVertices[1], // 3
		uniqueVertices[2],uniqueVertices[6],uniqueVertices[0], // 3
		uniqueVertices[7],uniqueVertices[6],uniqueVertices[2], // 4
		uniqueVertices[4],uniqueVertices[7],uniqueVertices[2], // 4
		uniqueVertices[4],uniqueVertices[1],uniqueVertices[3], // 5
		uniqueVertices[1],uniqueVertices[4],uniqueVertices[2], // 5
		uniqueVertices[4],uniqueVertices[3],uniqueVertices[5], // 6
		uniqueVertices[4],uniqueVertices[5],uniqueVertices[7], // 6
	};
	std::vector<Vector3> faceNormals {
		Vector3(-1,0,0),Vector3(-1,0,0),
		Vector3(0,0,-1),Vector3(0,0,-1),
		Vector3(0,-1,0),Vector3(0,-1,0),
		Vector3(0,0,1),Vector3(0,0,1),
		Vector3(1,0,0),Vector3(1,0,0),
		Vector3(0,1,0),Vector3(0,1,0)
	};
	std::vector<::Vector2> uvs {
		::Vector2(0,1),::Vector2(1,1),::Vector2(1,0), // 1
		::Vector2(0,1),::Vector2(1,0),::Vector2(0,0), // 1
		::Vector2(0,0),::Vector2(1,1),::Vector2(1,0), // 2
		::Vector2(0,0),::Vector2(0,1),::Vector2(1,1), // 2
		::Vector2(0,1),::Vector2(1,0),::Vector2(0,0), // 3
		::Vector2(0,1),::Vector2(1,1),::Vector2(1,0), // 3
		::Vector2(0,0),::Vector2(0,1),::Vector2(1,1), // 4
		::Vector2(1,0),::Vector2(0,0),::Vector2(1,1), // 4
		::Vector2(0,0),::Vector2(1,1),::Vector2(1,0), // 5
		::Vector2(1,1),::Vector2(0,0),::Vector2(0,1), // 5
		::Vector2(1,1),::Vector2(1,0),::Vector2(0,0), // 6
		::Vector2(1,1),::Vector2(0,0),::Vector2(0,1) // 6
	};
	for(auto &uv : uvs)
		uv.y = 1.f -uv.y;
	for(auto i=decltype(verts.size()){0};i<verts.size();i+=3)
	{
		auto &n = faceNormals[i /3];
		mesh.AddVertex(::Vertex{verts[i],uvs[i],n});
		mesh.AddVertex(::Vertex{verts[i +1],uvs[i +1],n});
		mesh.AddVertex(::Vertex{verts[i +2],uvs[i +2],n});

		mesh.AddTriangle(static_cast<uint32_t>(i),static_cast<uint32_t>(i +1),static_cast<uint32_t>(i +2));
	}
	mesh.SetSkinTextureIndex(0);
	mesh.Update();
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,mesh.shared_from_this());
}

void Lua::ModelSubMesh::InitializeSphere(lua_State *l,::ModelSubMesh &mesh,const Vector3 &origin,float radius,uint32_t recursionLevel)
{
	auto &meshVerts = mesh.GetVertices();
	auto &triangles = mesh.GetTriangles();
	std::vector<Vector3> verts;
	IcoSphere::Create(origin,radius,verts,triangles,recursionLevel);
	meshVerts.reserve(verts.size());
	for(auto &v : verts)
	{
		meshVerts.push_back({});
		auto &meshVert = meshVerts.back();
		meshVert.position = v;
		auto &n = meshVert.normal = uvec::get_normal(v -origin);
		meshVert.uv = {umath::atan2(n.x,n.z) /(2.f *M_PI) +0.5f,n.y *0.5f +0.5f};
	}

	mesh.SetSkinTextureIndex(0);
	mesh.Update();
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,mesh.shared_from_this());
}

void Lua::ModelSubMesh::InitializeCylinder(lua_State *l,::ModelSubMesh &mesh,float startRadius,float length,uint32_t segmentCount)
{
	auto rot = uquat::create_look_rotation(uvec::FORWARD,uvec::UP);

	auto &meshVerts = mesh.GetVertices();
	auto &triangles = mesh.GetTriangles();
	std::vector<Vector3> verts;
	Geometry::GenerateTruncatedConeMesh({},startRadius,{0.f,0.f,1.f},length,startRadius,verts,&triangles,nullptr,segmentCount);
	meshVerts.reserve(verts.size());
	for(auto &v : verts)
	{
		meshVerts.push_back({});
		meshVerts.back().position = v;
		// TODO: uv coordinates, etc.
	}
	mesh.GenerateNormals();

	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,mesh.shared_from_this());
}

void Lua::ModelSubMesh::InitializeCone(lua_State *l,::ModelSubMesh &mesh,float startRadius,float length,float endRadius,uint32_t segmentCount)
{
	auto rot = uquat::create_look_rotation(uvec::FORWARD,uvec::UP);

	auto &meshVerts = mesh.GetVertices();
	auto &triangles = mesh.GetTriangles();
	std::vector<Vector3> verts;
	Geometry::GenerateTruncatedConeMesh({},startRadius,{0.f,0.f,1.f},length,endRadius,verts,&triangles,nullptr,segmentCount);
	meshVerts.reserve(verts.size());
	for(auto &v : verts)
	{
		meshVerts.push_back({});
		meshVerts.back().position = v;
		// TODO: uv coordinates, etc.
	}
	mesh.GenerateNormals();

	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,mesh.shared_from_this());
}

static void add_back_face(std::vector<uint16_t> &tris)
{
	for(auto idx : {tris.at(tris.size() -3),tris.at(tris.size() -1),tris.at(tris.size() -2)})
		tris.push_back(idx);
}
void Lua::ModelSubMesh::InitializeRing(lua_State *l,::ModelSubMesh &mesh,std::optional<float> innerRadius,float outerRadius,bool doubleSided,uint32_t segmentCount)
{
	if(innerRadius.has_value() && *innerRadius == 0.f)
		innerRadius = {};
	auto stepSize = umath::round(360.f /static_cast<float>(segmentCount));

	auto &verts = mesh.GetVertices();
	auto &triangles = mesh.GetTriangles();
	auto numVerts = segmentCount;
	if(innerRadius.has_value())
		numVerts *= 2;
	else
		++numVerts;
	verts.reserve(segmentCount +1);

	auto numTris = segmentCount *3;
	if(doubleSided)
		numTris *= 2;
	if(innerRadius.has_value())
		numTris *= 2;
	triangles.reserve(numTris);

	if(innerRadius.has_value() == false)
		verts.push_back({});
	for(uint32_t i=0;i<=(360 +(innerRadius.has_value() ? stepSize : 0));i+=stepSize)
	{
		auto rad = umath::deg_to_rad(i);
		if(innerRadius.has_value())
		{
			verts.push_back({});
			verts.back().position = Vector3{umath::sin(rad),0.f,umath::cos(rad)} **innerRadius;
		}
		verts.push_back({});
		verts.back().position = Vector3{umath::sin(rad),0.f,umath::cos(rad)} *outerRadius;
		if(i == 0u)
			continue;
		if(innerRadius.has_value() == false)
		{
			triangles.push_back(0);
			triangles.push_back(verts.size() -2);
			triangles.push_back(verts.size() -1);

			if(doubleSided)
				add_back_face(triangles);
			continue;
		}
		triangles.push_back(verts.size() -1);
		triangles.push_back(verts.size() -2);
		triangles.push_back(verts.size());
		if(doubleSided)
			add_back_face(triangles);

		triangles.push_back(verts.size() -3);
		triangles.push_back(verts.size() -2);
		triangles.push_back(verts.size() -1);
		if(doubleSided)
			add_back_face(triangles);
	}
	mesh.GenerateNormals();

	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,mesh.shared_from_this());
}

void Lua::ModelSubMesh::InitializeCircle(lua_State *l,::ModelSubMesh &mesh,float radius,bool doubleSided,uint32_t segmentCount)
{
	InitializeRing(l,mesh,{},radius,doubleSided,segmentCount);
}


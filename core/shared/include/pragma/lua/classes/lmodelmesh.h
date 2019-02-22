#ifndef __LMODELMESH_H__
#define __LMODELMESH_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/math/icosphere.h"
#include <luasystem.h>

class ModelMesh;
class ModelSubMesh;
struct VertexWeight;
struct Vertex;

namespace Lua
{
	namespace ModelMesh
	{
		DLLNETWORK void register_class(luabind::class_<std::shared_ptr<::ModelMesh>> &classDef);
		DLLNETWORK void GetVertexCount(lua_State *l,std::shared_ptr<::ModelMesh> &mdl);
		DLLNETWORK void GetTriangleVertexCount(lua_State *l,std::shared_ptr<::ModelMesh> &mdl);
		DLLNETWORK void GetTriangleCount(lua_State *l,std::shared_ptr<::ModelMesh> &mdl);
		DLLNETWORK void GetSubMeshes(lua_State *l,std::shared_ptr<::ModelMesh> &mdl);
		DLLNETWORK void AddSubMesh(lua_State *l,std::shared_ptr<::ModelMesh> &mdl,std::shared_ptr<::ModelSubMesh> &mesh);
		DLLNETWORK void Update(lua_State *l,std::shared_ptr<::ModelMesh> &mdl);
		DLLNETWORK void Update(lua_State *l,std::shared_ptr<::ModelMesh> &mdl,uint32_t flags);
		DLLNETWORK void GetBounds(lua_State *l,std::shared_ptr<::ModelMesh> &mdl);
		DLLNETWORK void GetCenter(lua_State *l,std::shared_ptr<::ModelMesh> &mdl);
		DLLNETWORK void SetCenter(lua_State *l,std::shared_ptr<::ModelMesh> &mdl,const Vector3 &center);
		DLLNETWORK void Centralize(lua_State *l,std::shared_ptr<::ModelMesh> &mdl);
		DLLNETWORK void Scale(lua_State *l,std::shared_ptr<::ModelMesh> &mdl,const Vector3 &scale);
	};
	namespace ModelSubMesh
	{
		template<class TModelSubMesh>
			void CreateBox(lua_State *l,const Vector3 &min,const Vector3 &max);
		template<class TModelSubMesh>
			void CreateSphere(lua_State *l,const Vector3 &origin,float radius,uint32_t recursionLevel=1);
		DLLNETWORK void register_class(luabind::class_<std::shared_ptr<::ModelSubMesh>> &classDef);
		DLLNETWORK void GetVertexCount(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void GetTriangleVertexCount(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void GetTriangleCount(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void GetTexture(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void GetVertices(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void GetTriangles(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void GetUVMapping(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void GetNormalMapping(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void GetVertexWeights(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void AddTriangle(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const ::Vertex &v1,const ::Vertex &v2,const ::Vertex &v3);
		DLLNETWORK void AddTriangle(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t a,uint32_t b,uint32_t c);
		DLLNETWORK void SetTexture(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t texture);
		DLLNETWORK void Update(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void Update(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t flags);
		DLLNETWORK void AddVertex(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const ::Vertex &v);
		DLLNETWORK void GetBounds(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void SetVertex(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const ::Vertex &v);
		DLLNETWORK void SetVertexPosition(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const Vector3 &pos);
		DLLNETWORK void SetVertexNormal(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const Vector3 &normal);
		DLLNETWORK void SetVertexUV(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const ::Vector2 &uv);
		DLLNETWORK void SetVertexAlpha(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const ::Vector2 &alpha);
		DLLNETWORK void SetVertexWeight(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx,const ::VertexWeight &weight);
		DLLNETWORK void GetVertex(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx);
		DLLNETWORK void GetVertexPosition(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx);
		DLLNETWORK void GetVertexNormal(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx);
		DLLNETWORK void GetVertexUV(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx);
		DLLNETWORK void GetVertexAlpha(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx);
		DLLNETWORK void GetVertexWeight(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,uint32_t idx);
		DLLNETWORK void GetCenter(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void Optimize(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void GenerateNormals(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void NormalizeUVCoordinates(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl);
		DLLNETWORK void ClipAgainstPlane(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const Vector3 &n,double d);
		DLLNETWORK void ClipAgainstPlane(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const Vector3 &n,double d,bool bSplitCoverMeshes);
		DLLNETWORK void ClipAgainstPlane(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const Vector3 &n,double d,bool bSplitCoverMeshes,luabind::object boneMatrices);
		DLLNETWORK void ApplyUVMapping(lua_State *l,std::shared_ptr<::ModelSubMesh> &mdl,const Vector3 &nu,const Vector3 &nv,uint32_t w,uint32_t h,float ou,float ov,float su,float sv);
		DLLNETWORK void ApplyUVMapping(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh,const std::shared_ptr<::Model> &mdl,const Vector3 &nu,const Vector3 &nv,float ou,float ov,float su,float sv);
		DLLNETWORK void Scale(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh,const Vector3 &scale);
	};
};

template<class TModelSubMesh>
	void Lua::ModelSubMesh::CreateBox(lua_State *l,const Vector3 &cmin,const Vector3 &cmax)
{
	auto min = cmin;
	auto max = cmax;
	uvec::to_min_max(min,max);
	auto mesh = std::make_shared<TModelSubMesh>();
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
		mesh->AddVertex(::Vertex{verts[i],uvs[i],n});
		mesh->AddVertex(::Vertex{verts[i +1],uvs[i +1],n});
		mesh->AddVertex(::Vertex{verts[i +2],uvs[i +2],n});

		mesh->AddTriangle(static_cast<uint32_t>(i),static_cast<uint32_t>(i +1),static_cast<uint32_t>(i +2));
	}
	mesh->SetTexture(0);
	mesh->Update();
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,mesh);
}

template<class TModelSubMesh>
	void Lua::ModelSubMesh::CreateSphere(lua_State *l,const Vector3 &origin,float radius,uint32_t recursionLevel)
{
	auto mesh = std::make_shared<TModelSubMesh>();
	auto &meshVerts = mesh->GetVertices();
	auto &triangles = mesh->GetTriangles();
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
	
	mesh->SetTexture(0);
	mesh->Update();
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,mesh);
}

#endif

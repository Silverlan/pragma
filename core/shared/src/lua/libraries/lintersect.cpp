/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lintersect.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/lua/classes/ldef_plane.h"
#include "pragma/lua/classes/ldef_model.h"
#include "luasystem.h"
#include <pragma/model/modelmesh.h>
#include <pragma/math/intersection.h>

luabind::object Lua::intersect::line_obb(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,const Vector3 &min,const Vector3 &max,bool precise,const umath::Transform &pose)
{
	auto dist = 0.f;
	float *pDist = nullptr;
	if(precise == true)
		pDist = &dist;
	auto r = Intersection::LineOBB(rayStart,rayDir,min,max,pDist,pose.GetOrigin(),pose.GetRotation());
	if(r == false || precise == false)
		return luabind::object{l,r};
	return luabind::object{l,dist};
}

static bool get_line_mesh_result(lua_State *l,const Intersection::LineMeshResult &res,bool precise,luabind::object &r0,luabind::object &r1)
{
	if(precise == false)
	{
		r0 = luabind::object{l,res.result == Intersection::Result::Intersect};
		return false;
	}
	r0 = luabind::object{l,res.result};
	r1 = luabind::newtable(l);

	r1["triIdx"] = res.triIdx;
	r1["hitPos"] = res.hitPos;
	r1["hitValue"] = res.hitValue;
	r1["barycentricCoords"] = Vector3{static_cast<float>(res.t),static_cast<float>(res.u),static_cast<float>(res.v)};
	return true;
}

void Lua::intersect::line_mesh(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,ModelSubMesh &mesh,luabind::object &r0,luabind::object &r1,bool precise,const umath::Transform &meshPose)
{
	auto &origin = meshPose.GetOrigin();
	auto &rot = meshPose.GetRotation();
	Intersection::LineMeshResult res {};
	Intersection::LineMesh(rayStart,rayDir,mesh,res,precise,&origin,&rot);
	get_line_mesh_result(l,res,precise,r0,r1);
}

void Lua::intersect::line_mesh(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,ModelMesh &mesh,luabind::object &r0,luabind::object &r1,bool precise,const umath::Transform &meshPose)
{
	auto &origin = meshPose.GetOrigin();
	auto &rot = meshPose.GetRotation();
	Intersection::LineMeshResult res {};
	Intersection::LineMesh(rayStart,rayDir,mesh,res,precise,&origin,&rot);
	if(get_line_mesh_result(l,res,precise,r0,r1) == false)
		return;
	r1["subMeshIdx"] = res.subMeshIdx;
}

void Lua::intersect::line_mesh(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,Model &mdl,uint32_t lod,luabind::object &r0,luabind::object &r1,bool precise,const umath::Transform &meshPose)
{
	Intersection::LineMeshResult res {};
	Intersection::LineMesh(rayStart,rayDir,mdl,res,precise,nullptr,lod,meshPose.GetOrigin(),meshPose.GetRotation());
	if(get_line_mesh_result(l,res,precise,r0,r1) == false)
		return;
	r1["subMeshIdx"] = res.subMeshIdx;
	r1["meshGroupIdx"] = res.meshGroupIndex;
	r1["meshIdx"] = res.meshIdx;
}

void Lua::intersect::line_mesh(lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,Model &mdl,luabind::table<> tBodyGroups,luabind::object &r0,luabind::object &r1,bool precise,const umath::Transform &meshPose)
{
	auto bodyGroups = Lua::table_to_vector<uint32_t>(l,tBodyGroups,4);
	Intersection::LineMeshResult res {};
	Intersection::LineMesh(rayStart,rayDir,mdl,res,precise,&bodyGroups,0,meshPose.GetOrigin(),meshPose.GetRotation());
	if(get_line_mesh_result(l,res,precise,r0,r1) == false)
		return;
	r1["subMeshIdx"] = res.subMeshIdx;
	r1["meshGroupIdx"] = res.meshGroupIndex;
	r1["meshIdx"] = res.meshIdx;
}

void Lua::intersect::line_aabb(lua_State *l,const Vector3 &start,const Vector3 &dir,const Vector3 &min,const Vector3 &max,luabind::object &outMin,luabind::object &outMax)
{
	float tMin,tMax;
	auto res = Intersection::LineAABB(start,dir,min,max,&tMin,&tMax) == Intersection::Result::Intersect;
	if(res == false)
	{
		outMin = luabind::object{l,false};
		return;
	}
	outMin = luabind::object{l,tMin};
	outMax = luabind::object{l,tMax};
}

luabind::object Lua::intersect::line_plane(lua_State *l,const Vector3 &origin,const Vector3 &dir,const Vector3 &n,float d)
{
	float t;
	bool b = Intersection::LinePlane(origin,dir,n,d,&t) == Intersection::Result::Intersect;
	if(b == false)
		return luabind::object{l,false};
	return luabind::object{l,t};
}

bool Lua::intersect::point_in_plane_mesh(lua_State *l,const Vector3 &vec,luabind::table<> planeTable)
{
	auto planes = Lua::table_to_vector<Plane>(l,planeTable,2);
	return Intersection::PointInPlaneMesh(vec,planes);
}

int Lua::intersect::sphere_in_plane_mesh(lua_State *l,const Vector3 &vec,float r,luabind::table<> planeTable)
{
	auto planes = Lua::table_to_vector<Plane>(l,planeTable,3);
	return Intersection::SphereInPlaneMesh(vec,r,planes);
}

int Lua::intersect::aabb_in_plane_mesh(lua_State *l,const Vector3 &min,const Vector3 &max,luabind::table<> planeTable)
{
	auto planes = Lua::table_to_vector<Plane>(l,planeTable,3);
	return Intersection::AABBInPlaneMesh(min,max,planes);
}

void Lua::intersect::line_triangle(lua_State *l,const Vector3 &lineOrigin,const Vector3 &lineDir,const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,luabind::object &outT,luabind::object &outUv,bool cull)
{
	double t,u,v;
	auto bIntersect = Intersection::LineTriangle(lineOrigin,lineDir,v0,v1,v2,t,u,v,cull);
	if(bIntersect == false)
	{
		outT = luabind::object{l,false};
		return;
	}
	outT = luabind::object{l,t};
	outUv = luabind::object{l,Vector2{u,v}};
}

luabind::object Lua::intersect::line_line(lua_State *l,const Vector2 &start0,const Vector2 &end0,const Vector2 &start1,const Vector2 &end1)
{
	auto result = Intersection::LineLine(start0,end0,start1,end1);
	if(result.has_value() == false)
		return luabind::object{l,false};
	return luabind::object{l,*result};
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/math/intersection.h"
#include <mathutil/uvec.h>
#include <algorithm>
#include <pragma/model/modelmesh.h>
#include <pragma/model/model.h>

bool Intersection::LineMesh(
	const Vector3 &_start,const Vector3 &_dir,Model &mdl,LineMeshResult &r,bool precise,const std::vector<uint32_t> *bodyGroups,uint32_t lod,
	const Vector3 &origin,const Quat &rot
)
{
	auto start = _start;
	auto dir = _dir;
	uvec::world_to_local(origin,rot,start);
	uvec::rotate(&dir,uquat::get_inverse(rot));
	
	r.precise = r.precise ? r.precise : std::make_shared<LineMeshResult::Precise>();
	std::vector<std::shared_ptr<ModelMesh>> meshes;
	auto hasFoundBetterCandidate = false;
	if(bodyGroups == nullptr)
	{
		meshes.clear();
		mdl.GetBodyGroupMeshes({},lod,meshes);
		for(auto i=decltype(meshes.size()){0u};i<meshes.size();++i)
		{
			auto &mesh = meshes.at(i);
			if(LineMesh(start,dir,*mesh,r,precise,nullptr,nullptr) == false)
				continue;
			hasFoundBetterCandidate = true;
			r.precise->meshGroup = mdl.GetMeshGroup(0);
			r.precise->meshGroupIndex = 0;
			r.precise->mesh = mesh;
			r.precise->meshIdx = i;
			if(precise == false && r.result == umath::intersection::Result::Intersect)
				return true;
		}
		return hasFoundBetterCandidate;
	}
	for(auto outMeshGroupIdx : *bodyGroups)
	{
		meshes.clear();
		mdl.GetBodyGroupMeshes({outMeshGroupIdx},lod,meshes);
		for(auto i=decltype(meshes.size()){0u};i<meshes.size();++i)
		{
			auto &mesh = meshes.at(i);
			if(LineMesh(start,dir,*mesh,r,precise,nullptr,nullptr) == false)
				continue;
			hasFoundBetterCandidate = true;
			r.precise->meshGroupIndex = outMeshGroupIdx;
			r.precise->meshGroup = mdl.GetMeshGroup(outMeshGroupIdx);
			r.precise->meshIdx = i;
			r.precise->mesh = mesh;
			if(precise == false && r.result == umath::intersection::Result::Intersect)
				return true;
		}
	}
	return hasFoundBetterCandidate;
}
bool Intersection::LineMesh(const Vector3 &start,const Vector3 &dir,Model &mdl,LineMeshResult &r,bool precise,uint32_t lod,const Vector3 &origin,const Quat &rot) {return LineMesh(start,dir,mdl,r,precise,nullptr,lod,origin,rot);}
bool Intersection::LineMesh(const Vector3 &start,const Vector3 &dir,Model &mdl,LineMeshResult &r,bool precise,const std::vector<uint32_t> &bodyGroups,const Vector3 &origin,const Quat &rot) {return LineMesh(start,dir,mdl,r,precise,&bodyGroups,0,origin,rot);}
bool Intersection::LineMesh(const Vector3 &start,const Vector3 &dir,Model &mdl,LineMeshResult &r,bool precise,const Vector3 &origin,const Quat &rot) {return LineMesh(start,dir,mdl,r,precise,0,origin,rot);}

bool Intersection::LineMesh(const Vector3 &_start,const Vector3 &_dir,ModelMesh &mesh,LineMeshResult &r,bool precise,const Vector3 *origin,const Quat *rot)
{
	auto start = _start;
	auto dir = _dir;
	if(origin != nullptr && rot != nullptr)
	{
		uvec::world_to_local(*origin,*rot,start);
		uvec::rotate(&dir,uquat::get_inverse(*rot));
	}
	
	r.precise = r.precise ? r.precise : std::make_shared<LineMeshResult::Precise>();
	auto &subMeshes = mesh.GetSubMeshes();
	auto hasFoundBetterCandidate = false;
	for(auto i=decltype(subMeshes.size()){0u};i<subMeshes.size();++i)
	{
		auto &subMesh = subMeshes.at(i);
		if(LineMesh(start,dir,*subMesh,r,precise,nullptr,nullptr) == false)
			continue;
		hasFoundBetterCandidate = true;
		r.precise->subMeshIdx = i;
		r.precise->subMesh = subMesh;
		if(precise == false && r.result == umath::intersection::Result::Intersect)
			return true;
	}
	return hasFoundBetterCandidate;
}

static bool is_better_candidate(umath::intersection::Result oldResult,umath::intersection::Result newResult,float *tOld=nullptr,float *tNew=nullptr)
{
	return newResult > oldResult || (tOld != nullptr && newResult == oldResult && tNew > tOld);
}
static bool is_better_candidate(umath::intersection::Result oldResult,umath::intersection::Result newResult,float tOld,float tNew)
{
	return is_better_candidate(oldResult,newResult,&tOld,&tNew);
}
bool Intersection::LineMesh(const Vector3 &_start,const Vector3 &_dir,ModelSubMesh &subMesh,LineMeshResult &r,bool precise,const Vector3 *origin,const Quat *rot)
{
	if(subMesh.GetTriangleCount() == 0)
		return false;
	auto start = _start;
	auto dir = _dir;
	if(origin != nullptr && rot != nullptr)
	{
		uvec::world_to_local(*origin,*rot,start);
		uvec::rotate(&dir,uquat::get_inverse(*rot));
	}

	Vector3 min,max;
	subMesh.GetBounds(min,max);
	if(uvec::distance_sqr(min,max) == 0.f)
		return false;
	for(uint8_t i=0;i<3;++i)
	{
		// If the mesh is flat on one plane, we'll inflate it slightly
		if(max[i] -min[i] < 0.001)
			max[i] = min[i] +0.001;
	}
	auto tBounds = 0.f;
	if(umath::intersection::line_aabb(start,dir,min,max,&tBounds) == umath::intersection::Result::NoIntersection)
		return false;
	
	r.precise = r.precise ? r.precise : std::make_shared<LineMeshResult::Precise>();
	auto &triangles = subMesh.GetTriangles();
	auto &verts = subMesh.GetVertices();
	auto bHit = false;
	auto hasFoundBetterCandidate = false;
	for(auto i=decltype(triangles.size()){0};i<triangles.size();i+=3)
	{
		auto &va = verts[triangles[i]].position;
		auto &vb = verts[triangles[i +1]].position;
		auto &vc = verts[triangles[i +2]].position;

		umath::Plane p {va,vb,vc};
		float tl;
		auto rCur = umath::intersection::line_plane(start,dir,p.GetNormal(),p.GetDistance(),&tl);
		if(is_better_candidate(r.result,rCur,r.hitValue,tl) == false)
			continue;
		double t,u,v;
		if(umath::intersection::line_triangle(start,dir,va,vb,vc,t,u,v,true) == false)
			continue;
		hasFoundBetterCandidate = true;
		r.result = rCur;
		bHit = true;
		r.precise->triIdx = i /3;
		r.hitValue = tl;
		r.hitPos = start +dir *static_cast<float>(r.hitValue);

		if(precise == false && r.result == umath::intersection::Result::Intersect)
			return true;
	}
	if(bHit == true)
	{
		if(r.precise)
			r.precise->subMesh = subMesh.shared_from_this();
		if(r.hitValue >= 0.f && r.hitValue <= 1.f)
			r.result = umath::intersection::Result::Intersect;
		else
			r.result = umath::intersection::Result::OutOfRange;
	}
	return hasFoundBetterCandidate;
}

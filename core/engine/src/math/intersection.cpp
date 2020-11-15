/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_engine.h"
#include "pragma/math/intersection.h"
#include <mathutil/uvec.h>
#include <algorithm>
#include <pragma/model/modelmesh.h>
#include <pragma/model/model.h>

const float EPSILON = 1.19209e-005f;

static bool is_better_t(float tOld,float tNew)
{
	return tOld == std::numeric_limits<float>::max() || (tNew < 0.f && tNew > tOld) || (tNew >= 0.f && tNew < tOld);
}

static bool operator>(Intersection::Result a,Intersection::Result b)
{
	switch(a)
	{
		case Intersection::Result::NoIntersection:
			return b == Intersection::Result::Intersect || b == Intersection::Result::OutOfRange;
		case Intersection::Result::OutOfRange:
			return b == Intersection::Result::Intersect;
		case Intersection::Result::Intersect:
			return b != Intersection::Result::Intersect;
	}
	return false;
}

static bool operator>=(Intersection::Result a,Intersection::Result b) {return a > b || a == b;}
static bool operator<(Intersection::Result a,Intersection::Result b) {return !operator>=(a,b);}
static bool operator<=(Intersection::Result a,Intersection::Result b) {return a < b || a == b;}

static bool is_better_candidate(Intersection::Result oldResult,Intersection::Result newResult,float *tOld=nullptr,float *tNew=nullptr)
{
	return newResult > oldResult || (tOld != nullptr && newResult == oldResult && tNew > tOld);
}
static bool is_better_candidate(Intersection::Result oldResult,Intersection::Result newResult,float tOld,float tNew)
{
	return is_better_candidate(oldResult,newResult,&tOld,&tNew);
}

DLLENGINE bool Intersection::VectorInBounds(const Vector3 &vec,const Vector3 &min,const Vector3 &max,float EPSILON)
{
	if(EPSILON == 0.f)
		return vec.x >= min.x && vec.y >= min.y && vec.z >= min.z
			&& vec.x <= max.x && vec.y <= max.y && vec.z <= max.z;
	return vec.x >= min.x -EPSILON && vec.y >= min.y -EPSILON && vec.z >= min.z -EPSILON
		&& vec.x <= max.x +EPSILON && vec.y <= max.y +EPSILON && vec.z <= max.z +EPSILON;
}

DLLENGINE bool Intersection::SphereSphere(const Vector3 &originA,float rA,const Vector3 &originB,float rB)
{
	float dist = glm::distance(originA,originB);
	return dist -(rA +rB) <= 0;
}

DLLENGINE bool Intersection::AABBSphere(const Vector3 &min,const Vector3 &max,const Vector3 &origin,float r)
{
	Vector3 pClosest;
	Geometry::ClosestPointOnAABBToPoint(min,max,origin,&pClosest);
	float d = glm::distance(pClosest,origin);
	return d <= r;
}

bool Intersection::AABBInAABB(const Vector3 &minA,const Vector3 &maxA,const Vector3 &minB,const Vector3 &maxB)
{
	return (((minA.x >= minB.x && minA.y >= minB.y && minA.z >= minB.z) &&
		(maxA.x <= maxB.x && maxA.y <= maxB.y && maxA.z <= maxB.z))) ? true : false;
}

DLLENGINE Intersection::Intersect Intersection::AABBAABB(const Vector3 &minA,const Vector3 &maxA,const Vector3 &minB,const Vector3 &maxB)
{
	if((maxA.x < minB.x) ||
			(minA.x > maxB.x) ||
			(maxA.y < minB.y) ||
			(minA.y > maxB.y) ||
			(maxA.z < minB.z) ||
			(minA.z > maxB.z)
		)
		return Intersect::Outside;
	if(AABBInAABB(minA,maxA,minB,maxB) || AABBInAABB(minB,maxB,minA,maxA))
		return Intersect::Inside;
	return Intersect::Overlap;
}

DLLENGINE bool Intersection::AABBAABB(const AABB &a,const AABB &b)
{
	Vector3 t = b.pos -a.pos;
	return fabs(t.x) <= (a.extents.x +b.extents.x)
		&& fabs(t.y) <= (a.extents.y +b.extents.y)
		&& fabs(t.z) <= (a.extents.z +b.extents.z);
}

DLLENGINE bool Intersection::AABBTriangle(const Vector3 &min,const Vector3 &max,const Vector3 &a,const Vector3 &b,const Vector3 &c)
{
	Vector3 center = (min +max) *0.5f;
	min -= center;
	max -= center;
	a -= center;
	b -= center;
	c -= center;
	Vector3 minTri = a;
	Vector3 maxTri = a;
	uvec::min(&minTri,b);
	uvec::min(&minTri,c);
	uvec::max(&maxTri,b);
	uvec::max(&maxTri,c);
	if(AABBAABB(min,max,minTri,maxTri) == Intersect::Outside)
		return false;
	// TODO
	return true;
}

bool Intersection::OBBPlane(const Vector3 &min,const Vector3 &max,const Vector3 &origin,const Quat &rot,const Vector3 &n,double d)
{
	const std::array<Vector3,8> points = {
		min,
		{max.x,min.y,min.z},
		{min.x,max.y,min.z},
		{min.x,min.y,max.z},
		{max.x,max.y,min.z},
		{min.x,max.y,max.z},
		{max.x,min.y,max.z},
		max
	};
	uint8_t hitFlags = 0;
	for(auto p : points)
	{
		uvec::rotate(&p,rot);
		p += origin;
		auto pProj = uvec::project_to_plane(p,n,d);
		auto dot = uvec::dot(n,p -pProj);
		if(dot < 0.f)
			hitFlags |= 1;
		else if(dot > 0.f)
			hitFlags |= 2;
		else
			return true; // Point is on plane
		if(hitFlags == (1 | 2))
			return true;
	}
	return false;
}
bool Intersection::AABBPlane(const Vector3 &min,const Vector3 &max,const Vector3 &n,double d) {return OBBPlane(min,max,{},{},n,d);}
bool Intersection::SpherePlane(const Vector3 &sphereOrigin,float sphereRadius,const Vector3 &n,double d)
{
	auto pProj = uvec::project_to_plane(sphereOrigin,n,d);
	auto distSqr = uvec::length_sqr(sphereOrigin -pProj);
	return (distSqr <= umath::pow2(d)) ? true : false;
}

Intersection::Result Intersection::LineAABB(const Vector3 &o,const Vector3 &d,const Vector3 &min,const Vector3 &max,float *tMinRes,float *tMaxRes)
{
	Vector3 dirInv(1 /d.x,1 /d.y,1 /d.z);
	const int sign[] = {dirInv.x < 0,dirInv.y < 0,dirInv.z < 0};
	Vector3 bounds[] = {min,max};
	float tMin = (bounds[sign[0]].x -o.x) *dirInv.x;
	float tMax = (bounds[1 -sign[0]].x -o.x) *dirInv.x;
	float tyMin = (bounds[sign[1]].y -o.y) *dirInv.y;
	float tyMax = (bounds[1 -sign[1]].y -o.y) *dirInv.y;
	if((tMin > tyMax) || (tyMin > tMax))
		return Result::NoIntersection;
	if(tyMin > tMin)
		tMin = tyMin;
	if(tyMax < tMax)
		tMax = tyMax;
	float tzMin = (bounds[sign[2]].z -o.z) *dirInv.z;
	float tzMax = (bounds[1 -sign[2]].z -o.z) *dirInv.z;
	if((tMin > tzMax) || (tzMin > tMax))
		return Result::NoIntersection;
	if(tzMin > tMin)
		tMin = tzMin;
	if(tzMax < tMax)
		tMax = tzMax;
	if(tMinRes != nullptr)
		*tMinRes = tMin;
	if(tMaxRes != NULL)
		*tMaxRes = tMax;
	return Result::Intersect;
}

Intersection::Result Intersection::LinePlane(const Vector3 &o,const Vector3 &dir,const Vector3 &nPlane,float distPlane,float *t)
{
	float f = glm::dot(nPlane,dir);
	if(f == 0.f)
		return Result::NoIntersection;
	float hit = (glm::dot(nPlane,(nPlane *-distPlane) -o)) /f;
	if(t != NULL)
		*t = hit;
	return (hit >= 0.f && hit <= 1.f) ? Result::Intersect : Result::OutOfRange;
}

static bool point_in_triangle(const Vector3 &p,const Vector3 &a,const Vector3 &b,const Vector3 &c,const Vector3 origin={},const Quat rot=uquat::identity())
{
	std::array<Vector3,3> tri = {a,b,c};
	for(auto &v : tri)
	{
		uvec::rotate(&v,rot);
		v += origin;

		v = p -v;
		uvec::normalize(&v);
	}
	auto angles = umath::acos(uvec::dot(tri[0],tri[1])) +umath::acos(uvec::dot(tri[1],tri[2])) +umath::acos(uvec::dot(tri[2],tri[0]));
	return (umath::abs(angles -2.f *M_PI) <= 0.005f) ? true : false;
}

bool Intersection::LineOBB(const Vector3 &rayStart,const Vector3 &rayDir,const Vector3 &min,const Vector3 &max,float *dist,const Vector3 &origin,const Quat &rot)
{
	// Source: http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/
	auto tMin = 0.f;
	auto tMax = std::numeric_limits<float>::max();
	if(dist != nullptr)
		*dist = 0.f;

	auto t = umat::identity();
	t = glm::translate(t,origin);
	t *= umat::create(rot);

	auto obbPos = Vector3{t[3].x,t[3].y,t[3].z};
	auto delta = obbPos -rayStart;

	{
		Vector3 xaxis{t[0].x,t[0].y,t[0].z};
		auto e = uvec::dot(xaxis,delta);
		auto f = uvec::dot(rayDir,xaxis);

		if(umath::abs(f) > 0.001f)
		{
			auto t1 = (e +min.x) /f;
			auto t2 = (e +max.x) /f;

			if(t1 > t2)
			{
				auto w = t1;
				t1 = t2;
				t2 = w;
			}

			if(t2 < tMax)
				tMax = t2;
			if(t1 > tMin)
				tMin = t1;

			if(tMax < tMin)
				return false;
		}
		else if(-e +min.x > 0.f || -e +max.x < 0.f)
			return false;
	}

	{
		Vector3 yaxis{t[1].x,t[1].y,t[1].z};
		auto e = uvec::dot(yaxis,delta);
		auto f = uvec::dot(rayDir,yaxis);

		if(umath::abs(f) > 0.001f)
		{
			auto t1 = (e +min.y) /f;
			auto t2 = (e +max.y) /f;

			if(t1 > t2)
			{
				auto w = t1;
				t1 = t2;
				t2 = w;
			}

			if(t2 < tMax)
				tMax = t2;
			if(t1 > tMin)
				tMin = t1;
			if(tMin > tMax)
				return false;
		}
		else if(-e +min.y > 0.f || -e +max.y < 0.f)
			return false;
	}

	{
		Vector3 zaxis{t[2].x,t[2].y,t[2].z};
		auto e = uvec::dot(zaxis,delta);
		auto f = uvec::dot(rayDir,zaxis);

		if(umath::abs(f) > 0.001f)
		{
			auto t1 = (e +min.z) /f;
			auto t2 = (e +max.z) /f;

			if(t1 > t2)
			{
				auto w = t1;
				t1 = t2;
				t2 = w;
			}

			if(t2 < tMax)
				tMax = t2;
			if(t1 > tMin)
				tMin = t1;
			if(tMin > tMax)
				return false;
		}
		else if(-e +min.z > 0.f || -e +max.z < 0.f)
			return false;
	}
	if(dist != nullptr)
		*dist = tMin;
	return (tMin <= 1.f && tMin >= 0.f) ? true : false;
}

bool Intersection::LineMesh(
	const Vector3 &_start,const Vector3 &_dir,Model &mdl,LineMeshResult &r,bool precise,const std::vector<uint32_t> *bodyGroups,uint32_t lod,
	const Vector3 &origin,const Quat &rot
)
{
	auto start = _start;
	auto dir = _dir;
	uvec::world_to_local(origin,rot,start);
	uvec::rotate(&dir,uquat::get_inverse(rot));

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
			r.meshGroupIndex = 0;
			r.meshIdx = i;
			if(precise == false && r.result == Result::Intersect)
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
			r.meshGroupIndex = outMeshGroupIdx;
			r.meshIdx = i;
			if(precise == false && r.result == Result::Intersect)
				return true;
		}
	}
	return hasFoundBetterCandidate;
}
bool Intersection::LineMesh(const Vector3 &start,const Vector3 &dir,Model &mdl,LineMeshResult &r,bool precise,uint32_t lod,const Vector3 &origin,const Quat &rot) {return LineMesh(start,dir,mdl,r,precise,nullptr,lod,origin,rot);}
bool Intersection::LineMesh(const Vector3 &start,const Vector3 &dir,Model &mdl,LineMeshResult &r,bool precise,const std::vector<uint32_t> &bodyGroups,const Vector3 &origin,const Quat &rot) {return LineMesh(start,dir,mdl,r,precise,&bodyGroups,0,origin,rot);}
bool Intersection::LineMesh(const Vector3 &start,const Vector3 &dir,Model &mdl,LineMeshResult &r,bool precise,const Vector3 &origin,const Quat &rot) {return LineMesh(start,dir,mdl,r,precise,0,origin,rot);}

bool Intersection::LineSphere(const Vector3 &lineOrigin,const Vector3 &lineDir,const Vector3 &sphereOrigin,float sphereRadius,float &outT,Vector3 &outP)
{
	// Source: https://gamedev.stackexchange.com/a/96487/49279
	auto m = lineOrigin -sphereOrigin;
	float b = uvec::dot(m,lineDir);
	float c = uvec::dot(m,m) -umath::pow2(sphereRadius);

	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
	if(c > 0.0f && b > 0.0f)
		return false;
	float discr = b*b - c;

	// A negative discriminant corresponds to ray missing sphere 
	if(discr < 0.0f) return 0;

	// Ray now found to intersect sphere, compute smallest t value of intersection
	outT = -b - umath::sqrt(discr);

	// If t is negative, ray started inside sphere so clamp t to zero 
	if(outT < 0.0f) outT = 0.0f;
	outP = lineOrigin +outT *lineDir;
	return true;
}

bool Intersection::LineMesh(const Vector3 &_start,const Vector3 &_dir,ModelMesh &mesh,LineMeshResult &r,bool precise,const Vector3 *origin,const Quat *rot)
{
	auto start = _start;
	auto dir = _dir;
	if(origin != nullptr && rot != nullptr)
	{
		uvec::world_to_local(*origin,*rot,start);
		uvec::rotate(&dir,uquat::get_inverse(*rot));
	}

	auto &subMeshes = mesh.GetSubMeshes();
	auto hasFoundBetterCandidate = false;
	for(auto i=decltype(subMeshes.size()){0u};i<subMeshes.size();++i)
	{
		auto &subMesh = subMeshes.at(i);
		if(LineMesh(start,dir,*subMesh,r,precise,nullptr,nullptr) == false)
			continue;
		hasFoundBetterCandidate = true;
		r.subMeshIdx = i;
		if(precise == false && r.result == Result::Intersect)
			return true;
	}
	return hasFoundBetterCandidate;
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
	if(LineAABB(start,dir,min,max,&tBounds) == Result::NoIntersection)
		return false;

	auto &triangles = subMesh.GetTriangles();
	auto &verts = subMesh.GetVertices();
	auto bHit = false;
	auto hasFoundBetterCandidate = false;
	for(auto i=decltype(triangles.size()){0};i<triangles.size();i+=3)
	{
		auto &va = verts[triangles[i]].position;
		auto &vb = verts[triangles[i +1]].position;
		auto &vc = verts[triangles[i +2]].position;

		Plane p {va,vb,vc};
		float tl;
		auto rCur = LinePlane(start,dir,p.GetNormal(),p.GetDistance(),&tl);
		if(is_better_candidate(r.result,rCur,r.hitValue,tl) == false)
			continue;
		double t,u,v;
		if(LineTriangle(start,dir,va,vb,vc,t,u,v,true) == false)
			continue;
		hasFoundBetterCandidate = true;
		r.result = rCur;
		bHit = true;
		r.triIdx = i /3;
		r.hitValue = tl;
		r.hitPos = start +dir *static_cast<float>(r.hitValue);

		if(precise == false && r.result == Result::Intersect)
			return true;
	}
	if(bHit == true)
	{
		if(r.hitValue >= 0.f && r.hitValue <= 1.f)
			r.result = Result::Intersect;
		else
			r.result = Result::OutOfRange;
	}
	return hasFoundBetterCandidate;
}

DLLENGINE bool Intersection::PointInPlaneMesh(const Vector3 &vec,const std::vector<Plane> &planes)
{
	for(unsigned int i=0;i<planes.size();i++)
	{
		if(planes[i].GetDistance(vec) < 0.f)
			return false;
	}
	return true;
}

DLLENGINE Intersection::Intersect Intersection::SphereInPlaneMesh(const Vector3 &vec,float radius,const std::vector<Plane> &planes,bool skipInsideTest)
{
	if(PointInPlaneMesh(vec,planes) == false)
	{
		for(auto it=planes.begin();it!=planes.end();++it)
		{
			auto &plane = const_cast<Plane&>(*it);
			Vector3 p = vec +plane.GetNormal() *radius; // Closest point on sphere to plane
			if(plane.GetDistance(p) < 0)
				return Intersect::Outside;
		}
		return Intersect::Overlap;
	}
	if(skipInsideTest == true)
		return Intersect::Overlap;
	auto radiusSqr = umath::pow(radius,2.f);
	for(auto it=planes.begin();it!=planes.end();++it)
	{
		auto &plane = const_cast<Plane&>(*it);
		Vector3 r;
		Geometry::ClosestPointOnPlaneToPoint(plane.GetNormal(),CFloat(-plane.GetDistance()),vec,&r);
		if(uvec::length_sqr(r -vec) < radiusSqr)
			return Intersect::Overlap;
	}
	return Intersect::Inside;
}

DLLENGINE Intersection::Intersect Intersection::AABBInPlaneMesh(const Vector3 &min,const Vector3 &max,const std::vector<Plane> &planes)
{
	// Note: If the current method causes problems, try switching to the other one.
	// The second method is faster for most cases.
#define AABB_PLANE_MESH_INTERSECTION_METHOD 1
#if AABB_PLANE_MESH_INTERSECTION_METHOD == 0
	// Source: https://www.gamedev.net/forums/topic/672043-perfect-aabb-frustum-intersection-test/?do=findComment&comment=5254253
	UInt result = INTERSECT_INSIDE;
	
	for(auto &plane : planes)
	{
		// planes have unit-length normal, offset = -dot(normal, point on plane)
		const auto &n = plane.GetNormal();
		auto d = plane.GetDistance();
		auto nx = n.x > double(0);
		auto ny = n.y > double(0);
		auto nz = n.z > double(0);
		
		// getMinMax(): 0 = return min coordinate. 1 = return max.
		auto getMinMax = [&min,&max](bool v) -> Vector3 {
			return v ? max : min;
		};
		auto dot = (n.x*getMinMax(nx).x) + (n.y*getMinMax(ny).y) + (n.z*getMinMax(nz).z);
		
		if ( dot < -d )
			return INTERSECT_OUTSIDE;
		
		auto dot2 = (n.x*getMinMax(1-nx).x) + (n.y*getMinMax(1-ny).y) + (n.z*getMinMax(1-nz).z);
		
		if ( dot2 <= -d )
			result = INTERSECT_OVERLAP;
	}
	
	return result;
#else
	Vector3 vMin,vMax;
	auto r = Intersect::Inside;
	for(auto &plane : planes)
	{
		auto &n = plane.GetNormal();
		if(n.x > 0)
		{
			vMin.x = min.x; 
			vMax.x = max.x; 
		}
		else
		{ 
			vMin.x = max.x; 
			vMax.x = min.x; 
		}
		if(n.y > 0)
		{ 
			vMin.y = min.y; 
			vMax.y = max.y; 
		}
		else
		{ 
			vMin.y = max.y; 
			vMax.y = min.y; 
		}
		if(n.z > 0)
		{ 
			vMin.z = min.z; 
			vMax.z = max.z; 
		}
		else
		{ 
			vMin.z = max.z; 
			vMax.z = min.z; 
		} 
		if(plane.GetDistance(vMax) < 0)
			return Intersect::Outside;
		else if(plane.GetDistance(vMin) < 0)
			r = Intersect::Overlap;
	}
	return r;
#endif
}

bool Intersection::SphereCone(const Vector3 &sphereOrigin,float radius,const Vector3 &coneOrigin,const Vector3 &coneDir,float coneAngle)
{
	// Source: http://www.geometrictools.com/GTEngine/Include/Mathematics/GteIntrSphere3Cone3.h
	auto sinAngle = umath::sin(coneAngle);
	if(sinAngle == 0.f)
		return false;
	auto cosAngle = umath::cos(coneAngle);

	auto invSin = 1.f /sinAngle;
	auto cosSqr = cosAngle *cosAngle;
	auto CmV = sphereOrigin -coneOrigin;
	auto D = CmV +(radius *static_cast<float>(invSin)) *coneDir;
	auto lenSqr = uvec::dot(D,D);
	auto e = uvec::dot(D,coneDir);
	if(e > 0.f && e *e >= lenSqr *cosSqr)
	{
		auto sinSqr = sinAngle *sinAngle;
		lenSqr = uvec::dot(CmV,CmV);
		e = -uvec::dot(CmV,coneDir);
		if(e > 0.f && e *e >= lenSqr *sinSqr)
		{
			auto rSqr = radius *radius;
			return (lenSqr <= rSqr) ? true : false;
		}
		else
			return true;
	}
	return false;
}

// Source: http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
bool Intersection::LineTriangle(const Vector3 &lineOrigin,const Vector3 &lineDir,const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,double &t,double &u,double &v,bool bCull)
{
	const auto EPSILON = 0.000001;
	t = 0.0;
	u = 0.0;
	v = 0.0;
	auto edge1 = v1 -v0;
	auto edge2 = v2 -v0;
	auto pvec = uvec::cross(lineDir,edge2);

	auto det = uvec::dot(edge1,pvec);

	if(bCull == true)
	{
		if(det < EPSILON)
			return false;
		auto tvec = lineOrigin -v0;
		u = uvec::dot(tvec,pvec);
		if(u < 0.0 || u > det)
			return false;
		auto qvec = uvec::cross(tvec,edge1);
		v = uvec::dot(lineDir,qvec);
		if(v < 0.0 || (u +v) > det)
			return false;
		t = uvec::dot(edge2,qvec);
		auto invDet = 1.0 /det;
		t *= invDet;
		u *= invDet;
		v *= invDet;
	}
	else
	{
		if(det > -EPSILON && det < EPSILON)
			return false;
		auto invDet = 1.0 /det;
		auto tvec = lineOrigin -v0;
		u = uvec::dot(tvec,pvec) *invDet;
		if(u < 0.0 || u > 1.0)
			return false;
		auto qvec = uvec::cross(tvec,edge1);
		v = uvec::dot(lineDir,qvec) *invDet;
		if(v < 0.0 || (u +v) > 1.0)
			return false;
		t = uvec::dot(edge2,qvec) *invDet;
	}
	return true;
}

static float det(float a,float b,float c,float d)
{
	return a *d -b *c;
}

std::optional<Vector2> Intersection::LineLine(const Vector2 &start0,const Vector2 &end0,const Vector2 &start1,const Vector2 &end1)
{
	// http://mathworld.wolfram.com/Line-LineIntersection.html
	auto detL1 = det(start0.x,start0.y,end0.x,end0.y);
	auto detL2 = det(start1.x,start1.y,end1.x,end1.y);
	auto x1mx2 = start0.x -end0.x;
	auto x3mx4 = start1.x -end1.x;
	auto y1my2 = start0.y -end0.y;
	auto y3my4 = start1.y -end1.y;

	auto xnom = det(detL1,x1mx2,detL2,x3mx4);
	auto ynom = det(detL1,y1my2,detL2,y3my4);
	auto denom = det(x1mx2,y1my2,x3mx4,y3my4);
	if(denom == 0.0) // Lines don't seem to cross
		return {};

	if(umath::abs(denom) < 0.01)
		return {};

	auto ixOut = xnom /denom;
	auto iyOut = ynom /denom;
	return Vector2{ixOut,iyOut};
}

////////////////////////////////////

DLLENGINE void Geometry::ClosestPointOnAABBToPoint(const Vector3 &min,const Vector3 &max,const Vector3 &point,Vector3 *res)
{
	for(int i=0;i<3;i++)
	{
		float v = point[i];
		if(v < min[i]) v = min[i];
		if(v > max[i]) v = max[i];
		(*res)[i] = v;
	}
}

DLLENGINE void Geometry::ClosestPointOnPlaneToPoint(const Vector3 &n,float d,const Vector3 &p,Vector3 *res)
{
	float t = glm::dot(n,p) -d;
	*res = p -t *n;
}

DLLENGINE void Geometry::ClosestPointOnTriangleToPoint(const Vector3 &a,const Vector3 &b,const Vector3 &c,const Vector3 &p,Vector3 *res)
{
	Vector3 ab = b -a;
	Vector3 ac = c -a;
	Vector3 ap = p -a;
	float dA = glm::dot(ab,ap);
	float dB = glm::dot(ac,ap);
	if(dA <= 0.0f && dB <= 0.0f)
	{
		*res = a;
		return;
	}
	Vector3 bp = p -b;
	float dC = glm::dot(ab,bp);
	float dD = glm::dot(ac,bp);
	if(dC >= 0.0f && dD <= dC)
	{
		*res = b;
		return;
	}
	float vc = dA *dD -dC *dB;
	if(vc <= 0.0f && dA >= 0.0f && dC <= 0.0f)
	{
		float v = dA /(dA -dC);
		*res = a +v *ab;
		return;
	}
	Vector3 cp = p -c;
	float dE = glm::dot(ab,cp);
	float dF = glm::dot(ac,cp);
	if(dF >= 0.0f && dE <= dF)
	{
		*res = c;
		return;
	}
	float vb = dE *dB -dA *dF;
	if(vb <= 0.0f && dB >= 0.0f && dF <= 0.0f)
	{
		float w = dB /(dB -dF);
		*res = a +w *ac;
		return;
	}
	float va = dC *dF -dE *dD;
	if(va <= 0.0f && (dD -dC) >= 0.0f && (dE -dF) >= 0.0f)
	{
		float w = (dD -dC) /((dD -dC) +(dE -dF));
		*res = b +w *(c -b);
		return;
	}
	float denom = 1.0f /(va +vb +vc);
	float v = vb *denom;
	float w = vc *denom;
	*res = a +ab *v +ac *w;
}

DLLENGINE float Geometry::ClosestPointsBetweenLines(const Vector3 &pA,const Vector3 &qA,const Vector3 &pB,const Vector3 &qB,float *s,float *t,Vector3 *cA,Vector3 *cB)
{
	Vector3 dA = qA -pA;
	Vector3 dB = qB -pB;
	Vector3 r = pA -pB;
	float a = glm::dot(dA,dA);
	float e = glm::dot(dB,dB);
	float f = glm::dot(dB,r);
	float &rs = *s;
	float &rt = *t;
	if(a <= EPSILON && e <= EPSILON)
	{
		rs = rt = 0.0f;
		*cA = pA;
		*cB = pB;
		return glm::dot(*cA -(*cB),*cA -(*cB));
	}
	if(a <= EPSILON)
	{
		rs = 0.0f;
		rt = f /e;
		rt = glm::clamp(rt,0.0f,1.0f);
	}
	else
	{
		float c = glm::dot(dA,r);
		if(e <= EPSILON)
		{
			rt = 0.0f;
			rs = glm::clamp(-c /a,0.0f,1.0f);
		}
		else
		{
			float b = glm::dot(dA,dB);
			float denom = a *e -b *b;
			if(denom != 0.0f)
				rs = glm::clamp((b *f -c *e) /denom,0.0f,1.0f);
			else
				rs = 0.0f;
			rt = (b *rs +f) /e;
			if(rt < 0.0f)
			{
				rt = 0.0f;
				rs = glm::clamp(-c /a,0.0f,1.0f);
			}
			else if(rt > 1.0f)
			{
				rt = 1.0f;
				rs = glm::clamp((b -c) /a,0.0f,1.0f);
			}
		}
	}
	*cA = pA +dA *rs;
	*cB = pB +dB *rt;
	return glm::dot(*cA -(*cB),*cA -(*cB));
}

Vector3 Geometry::ClosestPointOnLineToPoint(const Vector3 &start,const Vector3 &end,const Vector3 &p,bool bClampResultToSegment)
{
	auto ap = p -start;
	auto ab = end -start;
	auto lenSqr = uvec::length_sqr(ab);
	if(lenSqr == 0.f)
		return start;
	auto dotAbap = uvec::dot(ap,ab);
	auto dist = dotAbap /lenSqr;
	if(bClampResultToSegment == true)
	{
		if(dist < 0.f)
			return start;
		else if(dist > 1.f)
			return end;
	}
	return start +ab *dist;
}

Vector3 Geometry::ClosestPointOnSphereToLine(const Vector3 &origin,float radius,const Vector3 &start,const Vector3 &end,bool bClampResultToSegment)
{
	auto pLine = ClosestPointOnLineToPoint(start,end,origin,bClampResultToSegment);
	auto dir = pLine -origin;
	auto l = uvec::length(dir);
	if(l == 0.f)
		return origin;
	dir /= l;
	if(l < radius)
		radius = l;
	return origin +dir *radius;
}

////////////////////////////////////

DLLENGINE bool Sweep::AABBWithAABB(const Vector3 &aa,const Vector3 &ab,const Vector3 &extA,
	const Vector3 &ba,const Vector3 &bb,const Vector3 &extB,
	float *entryTime,float *exitTime,Vector3 *normal)
{
	::AABB a(aa,extA);
	::AABB b(ba,extB);
	*entryTime = 0;
	*exitTime = 0;
	if(normal != NULL)
	{
		normal->x = 0;
		normal->y = 0;
		normal->z = 0;
	}
	if(a.Intersect(b))
		return true;
	Vector3 va = ab -aa;
	Vector3 vb = bb -ba;
	Vector3 invEntry(0,0,0);
	Vector3 invExit(0,0,0);
	Vector3 entry(0,0,0);
	Vector3 exit(0,0,0);
	Vector3 aMax = aa +extA;
	Vector3 bMax = ba +extB;
	aa -= extA;
	ba -= extB;
	for(int i=0;i<3;i++)
	{
		if(va[i] > 0.0f)
		{
			invEntry[i] = ba[i] -aMax[i];
			invExit[i] = bMax[i] -aa[i];
		}
		else
		{
			invEntry[i] = bMax[i] -aa[i];
			invExit[i] = ba[i] -aMax[i];
		}
		if(va[i] == 0.0f)
		{
			entry[i] = -std::numeric_limits<float>::infinity();
			exit[i] = std::numeric_limits<float>::infinity();
		}
		else
		{
			entry[i] = invEntry[i] /va[i];
			exit[i] = invExit[i] /va[i];
		}
	}
	float tEntry = std::max(entry.x,std::max(entry.y,entry.z));
	float tExit = std::min(exit.x,std::min(exit.y,exit.z));
	if(tEntry > tExit || entry.x < 0 && entry.y < 0 && entry.z < 0 || entry.x > 1 || entry.y > 1 || entry.z > 1)
		return false;
	if(entry.x > entry.y && entry.x > entry.z)
	{
		if(invEntry.x < 0)
		{
			normal->x = 1;
			normal->y = 0;
			normal->z = 0;
		}
		else
		{
			normal->x = -1;
			normal->y = 0;
			normal->z = 0;
		}
	}
	else if(entry.y > entry.x && entry.y > entry.z)
	{
		if(invEntry.y < 0)
		{
			normal->x = 0;
			normal->y = 1;
			normal->z = 0;
		}
		else
		{
			normal->x = 0;
			normal->y = -1;
			normal->z = 0;
		}
	}
	else
	{
		if(invEntry.z < 0)
		{
			normal->x = 0;
			normal->y = 0;
			normal->z = 1;
		}
		else
		{
			normal->x = 0;
			normal->y = 0;
			normal->z = -1;
		}
	}
	*entryTime = tEntry;
	*exitTime = tExit;
	return true;
}

DLLENGINE bool Sweep::AABBWithPlane(const Vector3 &origin,const Vector3 &dir,const Vector3 &ext,const Vector3 &planeNormal,float planeDistance,float *t)
{
	float r = ext.x *fabs(planeNormal.x) +ext.y *fabs(planeNormal.y) +ext.z *fabs(planeNormal.z);
	*t = (r -planeDistance -(glm::dot(planeNormal,origin))) /glm::dot(planeNormal,dir);
	if(*t < 0 || *t > 1)
		return false;
	return true;
}

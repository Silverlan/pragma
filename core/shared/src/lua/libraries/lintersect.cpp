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

int Lua::intersect::aabb_with_aabb(lua_State *l)
{
	Vector3 *minA = _lua_Vector_check(l,1);
	Vector3 *maxA = _lua_Vector_check(l,2);
	Vector3 *minB = _lua_Vector_check(l,3);
	Vector3 *maxB = _lua_Vector_check(l,4);
	auto r = Intersection::AABBAABB(*minA,*maxA,*minB,*maxB);
	Lua::PushInt(l,r);
	return 1;
}

int Lua::intersect::aabb_with_plane(lua_State *l)
{
	int32_t idx = 1;
	auto &min = *Lua::CheckVector(l,idx++);
	auto &max = *Lua::CheckVector(l,idx++);
	auto &n = *Lua::CheckVector(l,idx++);
	auto d = Lua::CheckNumber(l,idx++);
	auto r = Intersection::AABBPlane(min,max,n,d);
	Lua::PushBool(l,r);
	return 1;
}

int Lua::intersect::obb_with_plane(lua_State *l)
{
	int32_t idx = 1;
	auto &min = *Lua::CheckVector(l,idx++);
	auto &max = *Lua::CheckVector(l,idx++);
	auto &origin = *Lua::CheckVector(l,idx++);
	auto &rot = *Lua::CheckQuaternion(l,idx++);
	auto &n = *Lua::CheckVector(l,idx++);
	auto d = Lua::CheckNumber(l,idx++);
	auto r = Intersection::OBBPlane(min,max,origin,rot,n,d);
	Lua::PushBool(l,r);
	return 1;
}
int Lua::intersect::sphere_with_plane(lua_State *l)
{
	int32_t idx = 1;
	auto &sphereOrigin = *Lua::CheckVector(l,idx++);
	auto radius = Lua::CheckNumber(l,idx++);
	auto &n = *Lua::CheckVector(l,idx++);
	auto d = Lua::CheckNumber(l,idx++);
	auto r = Intersection::SpherePlane(sphereOrigin,radius,n,d);
	Lua::PushBool(l,r);
	return 1;
}

int Lua::intersect::sphere_with_sphere(lua_State *l)
{
	Vector3 *originA = _lua_Vector_check(l,1);
	float rA = Lua::CheckNumber<float>(l,2);
	Vector3 *originB = _lua_Vector_check(l,3);
	float rB = Lua::CheckNumber<float>(l,4);
	bool b = Intersection::SphereSphere(*originA,rA,*originB,rB);
	lua_pushboolean(l,b);
	return 1;
}

int Lua::intersect::aabb_with_sphere(lua_State *l)
{
	Vector3 *min = _lua_Vector_check(l,1);
	Vector3 *max = _lua_Vector_check(l,2);
	Vector3 *origin = _lua_Vector_check(l,3);
	float r = Lua::CheckNumber<float>(l,4);
	bool b = Intersection::AABBSphere(*min,*max,*origin,r);
	lua_pushboolean(l,b);
	return 1;
}

int Lua::intersect::line_obb(lua_State *l)
{
	int32_t arg = 1;
	auto *rayStart = Lua::CheckVector(l,arg++);
	auto *rayDir = Lua::CheckVector(l,arg++);
	auto *min = Lua::CheckVector(l,arg++);
	auto *max = Lua::CheckVector(l,arg++);
	auto bPrecise = false;
	Vector3 origin {};
	if(Lua::IsSet(l,arg) == true)
	{
		if(Lua::IsBool(l,arg) == true)
			bPrecise = Lua::CheckBool(l,arg++);
		else
			origin = *Lua::CheckVector(l,arg++);
	}
	auto rot = uquat::identity();
	if(Lua::IsSet(l,arg) == true)
	{
		if(Lua::IsBool(l,arg) == true)
			bPrecise = Lua::CheckBool(l,arg++);
		else
			rot = *Lua::CheckQuaternion(l,arg++);
	}
	if(Lua::IsSet(l,arg) == true)
		bPrecise = Lua::CheckBool(l,arg++);

	auto dist = 0.f;
	float *pDist = nullptr;
	if(bPrecise == true)
		pDist = &dist;
	auto r = Intersection::LineOBB(*rayStart,*rayDir,*min,*max,pDist,origin,rot);
	Lua::PushBool(l,r);
	if(bPrecise == true)
	{
		Lua::PushNumber(l,dist);
		return 2;
	}
	return 1;
}

int Lua::intersect::line_mesh(lua_State *l)
{
	int32_t arg = 1;
	auto *rayStart = Lua::CheckVector(l,arg++);
	auto *rayDir = Lua::CheckVector(l,arg++);

	uint8_t type = 0;
	void *tgt = nullptr;

	std::vector<uint32_t> bodyGroups;
	uint32_t lod = 0;
	if(Lua::IsType<ModelMesh>(l,arg) == true)
	{
		type = 0;
		tgt = &Lua::Check<ModelMesh>(l,arg++);
	}
	else if(Lua::IsType<ModelSubMesh>(l,arg) == true)
	{
		type = 1;
		tgt = &Lua::Check<ModelSubMesh>(l,arg++);
	}
	else
	{
		type = 2;
		tgt = &Lua::Check<Model>(l,arg++);

		if(Lua::IsTable(l,arg) == true)
		{
			Lua::PushValue(l,arg); /* 1 */
			Lua::PushNil(l); /* 2 */
			bodyGroups.reserve(Lua::GetObjectLength(l,arg));
			while(Lua::GetNextPair(l,-2))
			{
				auto bg = Lua::CheckInt(l,-1);
				bodyGroups.push_back(static_cast<uint32_t>(bg));
			} /* 1 */
			Lua::Pop(l,1); /* 0 */

			++arg;
		}
		if(Lua::IsNumber(l,arg))
			lod = static_cast<uint32_t>(Lua::CheckInt(l,arg++));
	}

	auto bPrecise = false;
	Vector3 origin {};
	if(Lua::IsSet(l,arg) == true)
	{
		if(Lua::IsBool(l,arg) == true)
			bPrecise = Lua::CheckBool(l,arg++);
		else
			origin = *Lua::CheckVector(l,arg++);
	}
	auto rot = uquat::identity();
	if(Lua::IsSet(l,arg) == true)
	{
		if(Lua::IsBool(l,arg) == true)
			bPrecise = Lua::CheckBool(l,arg++);
		else
			rot = *Lua::CheckQuaternion(l,arg++);
	}
	if(Lua::IsSet(l,arg) == true)
		bPrecise = Lua::CheckBool(l,arg++);

	auto t = 0.f;
	auto *pt = (bPrecise == true) ? &t : nullptr;
	auto r = Intersection::Result::NoIntersection;
	switch(type)
	{
		case 0:
		{
			r = Intersection::LineMesh(*rayStart,*rayDir,*static_cast<ModelMesh*>(tgt),&origin,&rot,pt);
			break;
		}
		case 1:
		{
			r = Intersection::LineMesh(*rayStart,*rayDir,*static_cast<ModelSubMesh*>(tgt),&origin,&rot,pt);
			break;
		}
		default:
		{
			r = Intersection::LineMesh(*rayStart,*rayDir,*static_cast<Model*>(tgt),bodyGroups,lod,origin,rot,pt);
			break;
		}
	}
	if(bPrecise == true)
	{
		Lua::PushInt(l,umath::to_integral(r));
		Lua::PushNumber(l,t);
		return 2;
	}
	else
		Lua::PushBool(l,r == Intersection::Result::Intersect);
	return 1;
}

int Lua::intersect::line_aabb(lua_State *l)
{
	Vector3 start = *_lua_Vector_check(l,1);
	Vector3 dir = *_lua_Vector_check(l,2);
	Vector3 min = *_lua_Vector_check(l,3);
	Vector3 max = *_lua_Vector_check(l,4);
	float tMin,tMax;
	bool b = Intersection::LineAABB(start,dir,min,max,&tMin,&tMax) == Intersection::Result::Intersect;
	lua_pushboolean(l,b);
	if(b)
	{
		Lua::PushNumber(l,tMin);
		Lua::PushNumber(l,tMax);
		return 3;
	}
	return 1;
}

int Lua::intersect::line_plane(lua_State *l)
{
	Vector3 origin = *_lua_Vector_check(l,1);
	Vector3 dir = *_lua_Vector_check(l,2);
	Vector3 n = *_lua_Vector_check(l,3);
	float d = Lua::CheckNumber<float>(l,4);
	float t;
	bool b = Intersection::LinePlane(origin,dir,n,d,&t) == Intersection::Result::Intersect;
	lua_pushboolean(l,b);
	if(b)
	{
		Lua::PushNumber(l,t);
		return 2;
	}
	return 1;
}

int Lua::intersect::vector_in_bounds(lua_State *l)
{
	Vector3 v = *_lua_Vector_check(l,1);
	Vector3 min = *_lua_Vector_check(l,2);
	Vector3 max = *_lua_Vector_check(l,3);
	bool b = Intersection::VectorInBounds(v,min,max);
	lua_pushboolean(l,b);
	return 1;
}

int Lua::intersect::point_in_plane_mesh(lua_State *l)
{
	Vector3 *vec = Lua::CheckVector(l,1);
	Lua::CheckTable(l,2);
	int table = Lua::GetStackTop(l);
	Lua::PushNil(l);
	std::vector<Plane> planes;
	while(Lua::GetNextPair(l,table) != 0)
	{
		Plane *plane = Lua::CheckPlane(l,-1);
		planes.push_back(*plane);
		Lua::Pop(l,1);
	}
	Lua::Pop(l,1);
	Lua::PushBool(l,Intersection::PointInPlaneMesh(*vec,planes));
	return 1;
}

int Lua::intersect::sphere_in_plane_mesh(lua_State *l)
{
	Vector3 *vec = Lua::CheckVector(l,1);
	float r = Lua::CheckNumber<float>(l,2);
	Lua::CheckTable(l,3);
	int table = Lua::GetStackTop(l);
	Lua::PushNil(l);
	std::vector<Plane> planes;
	while(Lua::GetNextPair(l,table) != 0)
	{
		Plane *plane = Lua::CheckPlane(l,-1);
		planes.push_back(*plane);
		Lua::Pop(l,1);
	}
	Lua::Pop(l,1);
	Lua::PushInt(l,Intersection::SphereInPlaneMesh(*vec,r,planes));
	return 1;
}

int Lua::intersect::aabb_in_plane_mesh(lua_State *l)
{
	Vector3 *min = Lua::CheckVector(l,1);
	Vector3 *max = Lua::CheckVector(l,2);
	Lua::CheckTable(l,3);
	int table = Lua::GetStackTop(l);
	Lua::PushNil(l);
	std::vector<Plane> planes;
	while(Lua::GetNextPair(l,table) != 0)
	{
		Plane *plane = Lua::CheckPlane(l,-1);
		planes.push_back(*plane);
		Lua::Pop(l,1);
	}
	Lua::Pop(l,1);
	Lua::PushInt(l,Intersection::AABBInPlaneMesh(*min,*max,planes));
	return 1;
}

int Lua::intersect::sphere_with_cone(lua_State *l)
{
	auto *sphereOrigin = Lua::CheckVector(l,1);
	auto radius = Lua::CheckNumber(l,2);
	auto *coneOrigin = Lua::CheckVector(l,3);
	auto *coneDir = Lua::CheckVector(l,4);
	auto coneAngle = Lua::CheckNumber(l,5);
	auto r = Intersection::SphereCone(*sphereOrigin,static_cast<float>(radius),*coneOrigin,*coneDir,static_cast<float>(coneAngle));
	Lua::PushBool(l,r);
	return 1;
}


int Lua::intersect::line_triangle(lua_State *l)
{
	auto &lineOrigin = *Lua::CheckVector(l,1);
	auto &lineDir = *Lua::CheckVector(l,2);
	auto &v0 = *Lua::CheckVector(l,3);
	auto &v1 = *Lua::CheckVector(l,4);
	auto &v2 = *Lua::CheckVector(l,5);
	auto bCull = false;
	if(Lua::IsSet(l,6))
		bCull = Lua::CheckBool(l,6);

	double t,u,v;
	auto bIntersect = Intersection::LineTriangle(lineOrigin,lineDir,v0,v1,v2,t,u,v,bCull);
	Lua::Push<bool>(l,bIntersect);
	Lua::PushNumber(l,t);
	Lua::Push<Vector2>(l,Vector2{u,v});
	return 3;
}

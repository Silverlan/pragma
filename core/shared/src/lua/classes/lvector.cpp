/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lvector.h"
#include <pragma/math/vector/wvvector3.h>
#include "luasystem.h"
#include "pragma/lua/classes/ldef_vector.h"
#include <sharedutils/util.h>
#include "pragma/model/vertex.h"
#include "pragma/lua/libraries/lmatrix.h"
#include "pragma/lua/classes/ldef_mat4.h"
#include "pragma/util/util_best_fitting_plane.hpp"
#include "pragma/math/util_engine_math.hpp"
#include <glm/gtx/projection.hpp>
#include <sharedutils/scope_guard.h>

::Vertex Lua::Vertex::Copy(::Vertex &v) {return v;}
::VertexWeight Lua::VertexWeight::Copy(::VertexWeight &vw) {return vw;}
::Vector3i Lua::Vectori::Copy(::Vector3i &v) {return v;}
::Vector2i Lua::Vector2i::Copy(::Vector2i &v) {return v;}
::Vector4i Lua::Vector4i::Copy(::Vector4i &v) {return v;}

void Lua::Vector::Normalize(Vector3 &vec) {uvec::normalize(&vec);}

static const Vector3 vPitch(1,0,0);
static const Vector3 vYaw(0,1,0);
static const Vector3 vRoll(0,0,1);
void Lua::Vector::Rotate(lua_State*,Vector3 &vec,const EulerAngles &ang) {uvec::rotate(&vec,ang);}
void Lua::Vector::Rotate(lua_State*,Vector3 &vec,const Vector3 &normal,float angle) {vec = glm::rotate(vec,angle,normal);}
void Lua::Vector::Rotate(lua_State*,Vector3 &vec,const Quat &orientation) {uvec::rotate(&vec,orientation);}
void Lua::Vector::RotateAround(lua_State*,Vector3 &vec,const Vector3 &origin,const EulerAngles &ang) {uvec::rotate_around(&vec,ang,origin);}
void Lua::Vector::Copy(lua_State *l,const Vector3 &vec) {luabind::object(l,vec).push(l);}
void Lua::Vector::Lerp(lua_State *l,const Vector3 &vec,const Vector3 &vecB,float factor) {luabind::object(l,uvec::lerp(vec,vecB,factor)).push(l);}

void Lua::Vector::Set(lua_State*,Vector3 &vec,const Vector3 &vecB)
{
	vec.x = vecB.x;
	vec.y = vecB.y;
	vec.z = vecB.z;
}

void Lua::Vector::Set(lua_State*,Vector3 &vec,float x,float y,float z)
{
	vec.x = x;
	vec.y = y;
	vec.z = z;
}

void Lua::Vector::ToMatrix(lua_State *l,const Vector3 &vec)
{
	auto m = glm::translate(umat::identity(),vec);
	luabind::object(l,m).push(l);
}

void Lua::Vector::SnapToGrid(lua_State*,Vector3 &vec,UInt32 gridSize)
{
	uvec::snap_to_grid(vec,gridSize);
}
void Lua::Vector::SnapToGrid(lua_State *l,Vector3 &vec)
{
	Lua::Vector::SnapToGrid(l,vec,1);
}

////////////////////////

Vector2 Lua::Vector2::GetNormal(const ::Vector2 &vec)
{
	Vector3 n(vec.x,vec.y,0);
	uvec::normalize(&n);
	return ::Vector2(n.x,n.y);
}
void Lua::Vector2::Normalize(::Vector2 &vec)
{
	Vector3 n(vec.x,vec.y,0);
	uvec::normalize(&n);
	vec.x = n.x;
	vec.y = n.y;
}
float Lua::Vector2::Length(const ::Vector2 &vec) {return uvec::length(Vector3(vec.x,vec.y,0));}
float Lua::Vector2::LengthSqr(const ::Vector2 &vec) {return uvec::length_sqr(Vector3(vec.x,vec.y,0));}
float Lua::Vector2::Distance(const ::Vector2 &a,const ::Vector2 &b)
{
	Vector3 v = Vector3(b.x,b.y,0) -Vector3(a.x,a.y,0);
	return uvec::length(v);
}
float Lua::Vector2::DistanceSqr(const ::Vector2 &a,const ::Vector2 &b)
{
	Vector3 v = Vector3(b.x,b.y,0) -Vector3(a.x,a.y,0);
	return uvec::length_sqr(v);
}
void Lua::Vector2::Cross(lua_State *l,const ::Vector2 &a,const ::Vector2 &b)
{
	Vector3 cross = glm::cross(Vector3(a.x,a.y,0),Vector3(b.x,b.y,0));
	luabind::object(l,::Vector2(cross.x,cross.y)).push(l);
}
void Lua::Vector2::DotProduct(lua_State *l,const ::Vector2 &a,const ::Vector2 &b)
{
	float d = glm::dot(Vector3(a.x,a.y,0),Vector3(b.x,b.y,0));
	Lua::PushNumber(l,d);
}
void Lua::Vector2::Rotate(lua_State*,::Vector2 &vec,const EulerAngles &ang)
{
	Vector3 v = Vector3(vec.x,vec.y,0);
	uvec::rotate(&v,ang);
	vec.x = v.x;
	vec.y = v.y;
}
void Lua::Vector2::RotateAround(lua_State*,::Vector2 &vec,const ::Vector2 &origin,const EulerAngles &ang)
{
	Vector3 v(vec.x,vec.y,0);
	Vector3 vB = Vector3(origin.x,origin.y,0);
	uvec::rotate_around(&v,ang,vB);
	vec.x = v.x;
	vec.y = v.y;
}
void Lua::Vector2::Copy(lua_State *l,const ::Vector2 &vec)
{
	luabind::object(l,::Vector2(vec.x,vec.y)).push(l);
}
void Lua::Vector2::Lerp(lua_State *l,const ::Vector2 &vec,const ::Vector2 &vecB,float factor)
{
	Vector3 v = uvec::lerp(Vector3(vec.x,vec.y,0),Vector3(vecB.x,vecB.y,0),factor);
	luabind::object(l,v).push(l);
}
void Lua::Vector2::Set(lua_State*,::Vector2 &vec,const ::Vector2 &vecB)
{
	vec.x = vecB.x;
	vec.y = vecB.y;
}
void Lua::Vector2::Set(lua_State*,::Vector2 &vec,float x,float y)
{
	vec.x = x;
	vec.y = y;
}
void Lua::Vector2::Project(lua_State *l,const ::Vector2 &vec,const ::Vector2 &n)
{
	Lua::Push<::Vector2>(l,glm::proj(vec,n));
}

////////////////////////

Vector4 Lua::Vector4::GetNormal(const ::Vector4 &vec)
{
	return glm::normalize(vec);
}
void Lua::Vector4::Normalize(::Vector4 &vec)
{
	vec = glm::normalize(vec);
}
float Lua::Vector4::Length(const ::Vector4 &vec)
{
	return glm::length(vec);
}
float Lua::Vector4::LengthSqr(const ::Vector4 &vec)
{
	return glm::length2(vec);
}
float Lua::Vector4::Distance(const ::Vector4 &a,const ::Vector4 &b)
{
	return glm::distance(a,b);
}
float Lua::Vector4::DistanceSqr(const ::Vector4 &a,const ::Vector4 &b)
{
	return glm::distance2(a,b);
}
void Lua::Vector4::DotProduct(lua_State *l,const ::Vector4 &a,const ::Vector4 &b)
{
	luabind::object(l,glm::dot(a,b)).push(l);
}
void Lua::Vector4::Copy(lua_State *l,const ::Vector4 &vec)
{
	luabind::object(l,::Vector4(vec.x,vec.y,vec.z,vec.w)).push(l);
}
void Lua::Vector4::Lerp(lua_State *l,const ::Vector4 &vec,const ::Vector4 &vecB,float factor)
{
	luabind::object(l,vec +(vecB -vec) *factor).push(l);
}
void Lua::Vector4::Set(lua_State*,::Vector4 &vec,const ::Vector4 &vecB)
{
	vec.w = vecB.w;
	vec.x = vecB.x;
	vec.y = vecB.y;
	vec.z = vecB.z;
}
void Lua::Vector4::Set(lua_State*,::Vector4 &vec,float x,float y,float z,float w)
{
	vec.w = w;
	vec.x = x;
	vec.y = y;
	vec.z = z;
}
void Lua::Vector4::Project(lua_State *l,const ::Vector4 &vec,const ::Vector4 &n)
{
	Lua::Push<::Vector4>(l,glm::proj(vec,n));
}

////////////////////////

template<class TVector>
	static void to_min_max(TVector &inOutA,TVector &inOutB)
{
	auto numComponents = inOutA.length();
	for(auto i=decltype(numComponents){0};i<numComponents;++i)
	{
		if(inOutB[i] < inOutA[i])
		{
			auto t = inOutA[i];
			inOutA[i] = inOutB[i];
			inOutB[i] = t;
		}
	}
}

template<class TVector>
	static void to_min_max(TVector &inOutA,TVector &inOutB,const TVector &c)
{
	auto numComponents = inOutA.length();
	for(auto i=decltype(numComponents){0};i<numComponents;++i)
	{
		if(c[i] < inOutA[i])
			inOutA[i] = c[i];
		if(c[i] > inOutB[i])
			inOutB[i] = c[i];
	}
}

void Lua::vector::to_min_max(::Vector2 &inOutA,::Vector2 &inOutB) {::to_min_max(inOutA,inOutB);}
void Lua::vector::to_min_max(::Vector3 &inOutA,::Vector3 &inOutB) {::to_min_max(inOutA,inOutB);}
void Lua::vector::to_min_max(::Vector4 &inOutA,::Vector4 &inOutB) {::to_min_max(inOutA,inOutB);}
void Lua::vector::to_min_max(::Vector2 &inOutA,::Vector2 &inOutB,const ::Vector2 &c) {::to_min_max(inOutA,inOutB,c);}
void Lua::vector::to_min_max(::Vector3 &inOutA,::Vector3 &inOutB,const ::Vector3 &c) {::to_min_max(inOutA,inOutB,c);}
void Lua::vector::to_min_max(::Vector4 &inOutA,::Vector4 &inOutB,const ::Vector4 &c) {::to_min_max(inOutA,inOutB,c);}

template<class TVector>
	static void get_min_max(lua_State *l,luabind::table<> t,TVector &outMin,TVector &outMax)
{
	auto numEls = Lua::GetObjectLength(l,1);
	if(numEls == 0)
	{
		outMin = {};
		outMax = {};
		return;
	}
	for(auto i=decltype(outMin.length()){0u};i<outMin.length();++i)
	{
		outMin[i] = std::numeric_limits<float>::max();
		outMax[i] = std::numeric_limits<float>::lowest();
	}
	for(auto it=luabind::iterator{t};it!=luabind::iterator{};++it)
	{
		auto val = luabind::object_cast_nothrow<TVector>(*it,TVector{});
		for(auto i=decltype(outMin.length()){0u};i<outMin.length();++i)
		{
			outMin[i] = umath::min(outMin[i],val[i]);
			outMax[i] = umath::min(outMax[i],val[i]);
		}
	}
}

void Lua::vector::get_min_max(lua_State *l,luabind::table<> t,::Vector2 &outMin,::Vector2 &outMax) {return ::get_min_max<::Vector2>(l,t,outMin,outMax);}
void Lua::vector::get_min_max(lua_State *l,luabind::table<> t,::Vector3 &outMin,::Vector3 &outMax) {return ::get_min_max<::Vector3>(l,t,outMin,outMax);}
void Lua::vector::get_min_max(lua_State *l,luabind::table<> t,::Vector4 &outMin,::Vector4 &outMax) {return ::get_min_max<::Vector4>(l,t,outMin,outMax);}

::Vector2 Lua::vector::random_2d()
{
	float azimuth = umath::random(0.f,2.f) *static_cast<float>(M_PI);
	return ::Vector2(std::cos(azimuth),std::sin(azimuth));
}

Vector3 Lua::vector::calc_average(luabind::table<> points)
{
	Vector3 avg {};
	uint32_t numPoints = 0;
	for(auto it=luabind::iterator{points};it!=luabind::iterator{};++it)
	{
		auto val = luabind::object_cast_nothrow<Vector3>(*it,Vector3{});
		avg += val;
		++numPoints;
	}
	avg /= static_cast<float>(numPoints);
	return avg;
}

void Lua::vector::calc_best_fitting_plane(luabind::table<> points,float ang,Vector3 &outNormal,double &outDistance)
{
	auto avg = calc_average(points);
	auto mat = umat::create_from_axis_angle(avg,ang);

	umath::calc_best_fitting_plane(mat,avg,outNormal,outDistance);
}

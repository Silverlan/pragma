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

void Lua::Vertex::Copy(lua_State *l,::Vertex &v)
{
	Lua::Push<::Vertex>(l,::Vertex{v});
}
void Lua::VertexWeight::Copy(lua_State *l,::VertexWeight &vw)
{
	Lua::Push<::VertexWeight>(l,::VertexWeight{vw});
}
void Lua::Vectori::Copy(lua_State *l,::Vector3i &v)
{
	Lua::Push<::Vector3i>(l,::Vector3i{v});
}
void Lua::Vector2i::Copy(lua_State *l,::Vector2i &v)
{
	Lua::Push<::Vector2i>(l,::Vector2i{v});
}
void Lua::Vector4i::Copy(lua_State *l,::Vector4i &v)
{
	Lua::Push<::Vector4i>(l,::Vector4i{v});
}

Vector3 Lua::Vector::GetNormal(const Vector3 &vec)
{
	Vector3 normal(vec);
	uvec::normalize(&normal);
	return normal;
}

void Lua::Vector::Normalize(Vector3 &vec) {uvec::normalize(&vec);}

EulerAngles Lua::Vector::Angle(const Vector3 &vec) {return uvec::to_angle(vec);}

float Lua::Vector::Length(const Vector3 &vec) {return uvec::length(vec);}

float Lua::Vector::LengthSqr(const Vector3 &vec) {return uvec::length_sqr(vec);}

float Lua::Vector::Distance(const Vector3 &a,const Vector3 &b) {return uvec::length(b -a);}
float Lua::Vector::DistanceSqr(const Vector3 &a,const Vector3 &b) {return uvec::length_sqr(b -a);}
float Lua::Vector::PlanarDistance(const Vector3 &a,const Vector3 &b,const Vector3 &n) {return uvec::planar_distance(a,b,n);}
float Lua::Vector::PlanarDistanceSqr(const Vector3 &a,const Vector3 &b,const Vector3 &n) {return uvec::planar_distance_sqr(a,b,n);}

void Lua::Vector::Cross(lua_State *l,const Vector3 &a,const Vector3 &b)
{
	Vector3 cross = glm::cross(a,b);
	luabind::object(l,cross).push(l);
}

void Lua::Vector::DotProduct(lua_State *l,const Vector3 &a,const Vector3 &b)
{
	float d = glm::dot(a,b);
	Lua::PushNumber(l,d);
}

void Lua::Vector::GetRotation(lua_State *l,const Vector3 &va,const Vector3 &vb)
{
	auto rot = uvec::get_rotation(va,vb);
	luabind::object(l,rot).push(l);
}

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
void Lua::Vector::Project(lua_State *l,const Vector3 &vec,const Vector3 &n)
{
	Lua::Push<Vector3>(l,uvec::project(vec,n));
}
void Lua::Vector::ProjectToPlane(lua_State *l,const Vector3 &p,const Vector3 &n,float d)
{
	Lua::Push<Vector3>(l,uvec::project_to_plane(p,n,d));
}
void Lua::Vector::GetPerpendicular(lua_State *l,const Vector3 &vec)
{
	Lua::Push<Vector3>(l,uvec::get_perpendicular(vec));
}
void Lua::Vector::OuterProduct(lua_State *l,const Vector3 &v0,const Vector3 &v1)
{
	Lua::Push<::Mat3>(l,uvec::calc_outer_product(v0,v1));
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

namespace Lua
{
	namespace vector
	{
		template<class TVector>
			int to_min_max(lua_State *l,TVector*(*f)(lua_State*,int));
	};
};

template<class TVector>
	int Lua::vector::to_min_max(lua_State *l,TVector*(*f)(lua_State*,int))
{
	auto &a = *f(l,1);
	auto numComponents = a.length();
	auto &b = *f(l,2);
	if(Lua::IsSet(l,3))
	{
		auto &c = *f(l,3);
		for(auto i=decltype(numComponents){0};i<numComponents;++i)
		{
			if(c[i] < a[i])
				a[i] = c[i];
			if(c[i] > b[i])
				b[i] = c[i];
		}
		return 0;
	}
	for(auto i=decltype(numComponents){0};i<numComponents;++i)
	{
		if(b[i] < a[i])
		{
			auto t = a[i];
			a[i] = b[i];
			b[i] = t;
		}
	}
	return 0;
}

int Lua::vector::to_min_max(lua_State *l)
{
	if(Lua::IsVector(l,1))
		return to_min_max(l,&Lua::CheckVector);
	if(Lua::IsVector2(l,1))
		return to_min_max(l,&Lua::CheckVector2);
	return to_min_max(l,&Lua::CheckVector4);
}

int Lua::vector::get_min_max(lua_State *l)
{
	int table = 1;
	Lua::CheckTable(l,table);
	Lua::PushNil(l);
	unsigned int idx = 0;
	if(Lua::GetNextPair(l,table) == 0)
	{
		Lua::Push<Vector3>(l,Vector3(0.f,0.f,0.f));
		Lua::Push<Vector3>(l,Vector3(0.f,0.f,0.f));
		return 2;
	}
	bool bVec4 = Lua::IsVector4(l,-1);
	bool bVec3 = Lua::IsVector(l,-1);
	bool bVec2 = Lua::IsVector2(l,-1);
	UNUSED(bVec2);
	if(bVec4)
	{
		::Vector4 min(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
		::Vector4 max(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest());
		do
		{
			::Vector4 *v = Lua::CheckVector4(l,-1);
			for(char i=0;i<4;i++)
			{
				if((*v)[i] < min[i])
					min[i] = (*v)[i];
				if((*v)[i] > max[i])
					max[i] = (*v)[i];
			}
			Lua::Pop(l,1);
			idx++;
		}
		while(Lua::GetNextPair(l,table) != 0);
		Lua::Push<::Vector4>(l,min);
		Lua::Push<::Vector4>(l,max);
		return 2;
	}
	if(bVec3)
	{
		Vector3 min(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
		Vector3 max(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest());
		do
		{
			Vector3 *v = Lua::CheckVector(l,-1);
			for(char i=0;i<3;i++)
			{
				if((*v)[i] < min[i])
					min[i] = (*v)[i];
				if((*v)[i] > max[i])
					max[i] = (*v)[i];
			}
			Lua::Pop(l,1);
			idx++;
		}
		while(Lua::GetNextPair(l,table) != 0);
		Lua::Push<Vector3>(l,min);
		Lua::Push<Vector3>(l,max);
		return 2;
	}
	::Vector2 min(std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
	::Vector2 max(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest());
	do
	{
		::Vector2 *v = Lua::CheckVector2(l,-1);
		for(char i=0;i<2;i++)
		{
			if((*v)[i] < min[i])
				min[i] = (*v)[i];
			if((*v)[i] > max[i])
				max[i] = (*v)[i];
		}
		Lua::Pop(l,1);
		idx++;
	}
	while(Lua::GetNextPair(l,table) != 0);
	Lua::Push<::Vector2>(l,min);
	Lua::Push<::Vector2>(l,max);
	return 2;
}

int Lua::vector::random_2d(lua_State *l)
{
	float azimuth = umath::random(0.f,2.f) *static_cast<float>(M_PI);
	luabind::object(l,::Vector2(std::cos(azimuth),std::sin(azimuth))).push(l);
	return 1;
}

int Lua::vector::create_from_string(lua_State *l)
{
	auto *str = Lua::CheckString(l,1);
	auto v = uvec::create(str);
	Lua::Push<Vector3>(l,v);
	return 1;
}

int Lua::vector::random(lua_State *l)
{
	luabind::object(l,uvec::create_random_unit_vector()).push(l);
	return 1;
}

int Lua::vector::calc_average(lua_State *l)
{
	Vector3 avg {};
	int32_t tPoints = 1;
	Lua::CheckTable(l,tPoints);
	auto numPoints = Lua::GetObjectLength(l,tPoints);
	for(auto i=decltype(numPoints){0};i<numPoints;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,tPoints);
		avg += *Lua::CheckVector(l,-1);
	}
	avg /= static_cast<float>(numPoints);
	Lua::Push<Vector3>(l,avg);
	return 1;
}

int Lua::vector::calc_best_fitting_plane(lua_State *l)
{
	if(calc_average(l) < 1)
		return 0;
	auto &avg = *Lua::CheckVector(l,-1);
	if(Lua::matrix::calc_covariance_matrix(l) < 1)
	{
		Lua::Pop(l,1); // Pop vector
		return 0;
	}
	auto &mat = *Lua::CheckMat3(l,-1);
	Vector3 n;
	double d;
	umath::calc_best_fitting_plane(mat,avg,n,d);

	Lua::Pop(l,2); // Pop vector and matrix

	Lua::Push<Vector3>(l,n);
	Lua::PushNumber(l,d);
	return 2;
}

int Lua::vector::angular_velocity_to_linear(lua_State *l)
{
	auto &refPos = *Lua::CheckVector(l,1);
	auto &angVel = *Lua::CheckVector(l,2);
	auto &tgtPos = *Lua::CheckVector(l,3);
	Lua::Push<Vector3>(l,util::angular_velocity_to_linear(refPos,angVel,tgtPos));
	return 1;
}

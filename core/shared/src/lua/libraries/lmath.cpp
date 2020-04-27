/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/libraries/lmath.h"
#include <pragma/engine.h>
#include <mathutil/umath.h>
#include "luasystem.h"
#include "pragma/math/perlinnoise.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/math/util_hermite.h"
#include "pragma/util/util_ballistic.h"
#include "pragma/math/util_easing.hpp"
#include "pragma/model/modelmesh.h"
#include <mathutil/umath_equation_solver.h>
#include <mathutil/umath_frustum.hpp>

extern DLLENGINE Engine *engine;
int Lua::math::approach(lua_State *l)
{
	auto val = Lua::CheckNumber(l,1);
	auto tgt = Lua::CheckNumber(l,2);
	auto inc = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,umath::approach(val,tgt,inc));
	return 1;
}

int Lua::math::randomf(lua_State *l)
{
	float min,max;
	if(lua_isnoneornil(l,1) && lua_isnoneornil(l,2))
	{
		min = 0;
		max = 1;
	}
	else
	{
		min = Lua::CheckNumber<float>(l,1);
		max = Lua::CheckNumber<float>(l,2);
	}
	Lua::PushNumber(l,umath::random(min,max));
	return 1;
}

int Lua::math::get_angle_difference(lua_State *l)
{
	float angA = Lua::CheckNumber<float>(l,1);
	float angB = Lua::CheckNumber<float>(l,2);
	Lua::PushNumber(l,umath::get_angle_difference(angA,angB));
	return 1;
}
int Lua::math::approach_angle(lua_State *l)
{
	float angA = Lua::CheckNumber<float>(l,1);
	float angB = Lua::CheckNumber<float>(l,2);
	float am = Lua::CheckNumber<float>(l,3);
	Lua::PushNumber(l,umath::approach_angle(angA,angB,am));
	return 1;
}
int Lua::math::normalize_angle(lua_State *l)
{
	float ang = Lua::CheckNumber<float>(l,1);
	if(Lua::IsSet(l,2))
	{
		float base = Lua::CheckNumber<float>(l,2);
		Lua::PushNumber(l,umath::normalize_angle(ang,base));
	}
	else
		Lua::PushNumber(l,umath::normalize_angle(ang));
	return 1;
}
int Lua::math::clamp_angle(lua_State *l)
{
	auto ang = Lua::CheckNumber<float>(l,1);
	auto min = Lua::CheckNumber<float>(l,2);
	auto max = Lua::CheckNumber<float>(l,3);
	Lua::PushNumber(l,umath::clamp_angle(ang,min,max));
	return 1;
}
int Lua::math::is_angle_in_range(lua_State *l)
{
	auto ang = Lua::CheckNumber<float>(l,1);
	auto min = Lua::CheckNumber<float>(l,2);
	auto max = Lua::CheckNumber<float>(l,3);
	Lua::PushBool(l,umath::is_angle_in_range(ang,min,max));
	return 1;
}

int Lua::math::perlin_noise(lua_State *l)
{
	Vector3 &v = *Lua::CheckVector(l,1);
	if(Lua::IsSet(l,2))
	{
		unsigned int seed = Lua::CheckInt<unsigned int>(l,2);
		PerlinNoise p(seed);
		Lua::PushNumber(l,p.GetNoise(v));
		return 1;
	}
	PerlinNoise p;
	Lua::PushNumber(l,p.GetNoise(v));
	return 1;
}

int Lua::math::sign(lua_State *l)
{
	double n = Lua::CheckNumber(l,1);
	if(n == 0)
	{
		Lua::PushInt(l,1);
		return 1;
	}
	Lua::PushInt(l,static_cast<int>(n /fabs(n)));
	return 1;
}

int Lua::math::clamp(lua_State *l)
{
	auto val = Lua::CheckNumber(l,1);
	auto min = Lua::CheckNumber(l,2);
	auto max = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,umath::clamp(val,min,max));
	return 1;
}

int Lua::math::lerp(lua_State *l)
{
	float a = Lua::CheckNumber<float>(l,1);
	float b = Lua::CheckNumber<float>(l,2);
	float f = Lua::CheckNumber<float>(l,3);
	Lua::PushNumber(l,a +f *(b -a));
	return 1;
}

int Lua::math::get_next_power_of_2(lua_State *l)
{
	int v = Lua::CheckInt<int>(l,1);
	Lua::PushInt(l,umath::next_power_of_2(v));
	return 1;
}

int Lua::math::get_previous_power_of_2(lua_State *l)
{
	int v = Lua::CheckInt<int>(l,1);
    Lua::PushInt(l,umath::previous_power_of_2(v));
	return 1;
}

static float round_by_multiple(float f,float multiple)
{
	return (multiple != 0.f) ? (::umath::round(f /multiple) *multiple) : f;
}

int Lua::math::round(lua_State *l)
{
	float f = Lua::CheckNumber<float>(l,1);
	if(Lua::IsSet(l,2))
	{
		auto multiple = Lua::CheckNumber(l,2);
		Lua::PushNumber(l,round_by_multiple(f,multiple));
		return 1;
	}
	int i = umath::round(f);
	Lua::PushInt(l,i);
	return 1;
}

int Lua::math::snap_to_grid(lua_State *l)
{
	auto f = Lua::CheckNumber(l,1);
	UInt32 gridSize = 1;
	if(Lua::IsSet(l,2))
		gridSize = static_cast<uint32_t>(Lua::CheckInt(l,2));
	auto r = umath::snap_to_grid(static_cast<float>(f),gridSize);
	Lua::PushInt(l,r);
	return 1;
}

int Lua::math::calc_hermite_spline(lua_State *l)
{
	auto &p0 = *Lua::CheckVector(l,1);
	auto &p1 = *Lua::CheckVector(l,2);
	auto &p2 = *Lua::CheckVector(l,3);
	auto &p3 = *Lua::CheckVector(l,4);
	auto segmentCount = Lua::CheckInt(l,5);
	auto curvature = 1.f;
	if(Lua::IsSet(l,6))
		curvature = static_cast<float>(Lua::CheckNumber(l,6));

	std::vector<Vector3> curvePoints {};
	util::calc_hermite_spline(p0,p1,p2,p3,static_cast<uint32_t>(segmentCount),curvePoints,curvature);
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(curvePoints.size()){0};i<curvePoints.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::Push<Vector3>(l,curvePoints[i]);
		Lua::SetTableValue(l,t);
	}
	return 1;
}

int Lua::math::calc_hermite_spline_position(lua_State *l)
{
	auto &p0 = *Lua::CheckVector(l,1);
	auto &p1 = *Lua::CheckVector(l,2);
	auto &p2 = *Lua::CheckVector(l,3);
	auto &p3 = *Lua::CheckVector(l,4);
	auto s = Lua::CheckNumber(l,5);
	auto curvature = 1.f;
	if(Lua::IsSet(l,6))
		curvature = static_cast<float>(Lua::CheckNumber(l,6));

	auto pos = util::calc_hermite_spline_position(p0,p1,p2,p3,static_cast<float>(s),curvature);
	Lua::Push<Vector3>(l,pos);
	return 1;
}

int Lua::math::smooth_step(lua_State *l)
{
	auto edge0 = Lua::CheckNumber(l,1);
	auto edge1 = Lua::CheckNumber(l,2);
	auto x = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,umath::smooth_step(edge0,edge1,x));
	return 1;
}
int Lua::math::smoother_step(lua_State *l)
{
	auto edge0 = Lua::CheckNumber(l,1);
	auto edge1 = Lua::CheckNumber(l,2);
	auto x = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,umath::smoother_step(edge0,edge1,x));
	return 1;
}
int Lua::math::is_in_range(lua_State *l)
{
	auto v = Lua::CheckNumber(l,1);
	auto min = Lua::CheckNumber(l,2);
	auto max = Lua::CheckNumber(l,3);
	Lua::PushBool(l,(v >= min && v <= max) ? true : false);
	return 1;
}
int Lua::math::normalize_uv_coordinates(lua_State *l)
{
	auto &uv = *Lua::CheckVector2(l,1);
	umath::normalize_uv_coordinates(uv);
	return 0;
}

template<int32_t nc,int32_t ns>
	int32_t solve_equation(lua_State *l,const std::function<int32_t(const std::array<double,nc>&,std::array<double,ns>&)> &fSolve)
{
	int32_t idx = 1;
	std::array<double,nc> c;
	for(auto i=decltype(c.size()){0};i<c.size();++i)
		c[i] = Lua::CheckNumber(l,idx++);

	std::array<double,ns> s;
	auto n = fSolve(c,s);
	Lua::PushInt(l,n);
	for(auto i=decltype(n){0};i<n;++i)
		Lua::PushNumber(l,s[i]);

	return n +1;
}

int Lua::math::solve_quadric(lua_State *l) {return solve_equation<3,2>(l,&umath::solve_quadric);}
int Lua::math::solve_cubic(lua_State *l) {return solve_equation<4,3>(l,&umath::solve_cubic);}
int Lua::math::solve_quartic(lua_State *l) {return solve_equation<5,4>(l,&umath::solve_quartic);}
int Lua::math::calc_ballistic_range(lua_State *l)
{
	auto speed = Lua::CheckNumber(l,1);
	auto gravity = Lua::CheckNumber(l,2);
	auto initialHeight = Lua::CheckNumber(l,3);
	auto r = umath::calc_ballistic_range(speed,gravity,initialHeight);
	Lua::PushNumber(l,r);
	return 1;
}
int Lua::math::calc_ballistic_position(lua_State *l)
{
	auto &start = *Lua::CheckVector(l,1);
	auto &vel = *Lua::CheckVector(l,2);
	auto gravity = Lua::CheckNumber(l,3);
	auto t = Lua::CheckNumber(l,4);

	auto r = umath::calc_ballistic_position(start,vel,gravity,t);
	Lua::Push<Vector3>(l,r);
	return 1;
}
int Lua::math::calc_ballistic_velocity(lua_State *l)
{
	auto &start = *Lua::CheckVector(l,1);
	auto &end = *Lua::CheckVector(l,2);
	auto angle = Lua::CheckNumber(l,3);
	auto gravity = Lua::CheckNumber(l,4);
	Vector3 vel;
	auto r = umath::calc_ballistic_velocity(start,end,angle,gravity,vel);
	if(r == false)
		return 0;
	Lua::Push<Vector3>(l,vel);
	return 1;
}
int Lua::math::calc_ballistic_time_of_flight(lua_State *l)
{
	int32_t argIdx = 1;
	auto &start = *Lua::CheckVector(l,argIdx++);
	auto *v0 = Lua::IsVector(l,argIdx) ? Lua::CheckVector(l,argIdx++) : nullptr;
	if(v0 == nullptr)
	{
		auto launchAngle = Lua::CheckNumber(l,argIdx++);
		auto velocity = Lua::CheckNumber(l,argIdx++);
		auto gravity = Lua::CheckNumber(l,argIdx++);

		auto t = umath::calc_ballistic_time_of_flight(start,launchAngle,velocity,gravity);
		Lua::PushNumber(l,t);
		return 1;
	}
	auto *v1 = Lua::IsVector(l,argIdx) ? Lua::CheckVector(l,argIdx++) : nullptr;
	if(v1 != nullptr)
	{
		auto gravity = Lua::CheckNumber(l,argIdx++);

		auto t = umath::calc_ballistic_time_of_flight(start,*v0,*v1,gravity);
		Lua::PushNumber(l,t);
		return 1;
	}
	if(Lua::IsSet(l,argIdx +1) == true)
	{
		auto launchAngle = Lua::CheckNumber(l,argIdx++);
		auto velocity = Lua::CheckNumber(l,argIdx++);
		auto gravity = Lua::CheckNumber(l,argIdx++);

		auto t = umath::calc_ballistic_time_of_flight(start,*v0,launchAngle,velocity,gravity);
		Lua::PushNumber(l,t);
		return 1;
	}
	auto gravity = Lua::CheckNumber(l,argIdx++);
	auto t = umath::calc_ballistic_time_of_flight(start,*v0,gravity);
	Lua::PushNumber(l,t);
	return 1;
}
int Lua::math::calc_ballistic_angle_of_reach(lua_State *l)
{
	auto &start = *Lua::CheckVector(l,1);
	auto distance = Lua::CheckNumber(l,2);
	auto initialVelocity = Lua::CheckNumber(l,3);
	auto gravity = Lua::CheckNumber(l,4);
	
	auto ang = umath::calc_ballistic_angle_of_reach(start,distance,initialVelocity,gravity);
	Lua::PushNumber(l,ang);
	return 1;
}
int Lua::math::solve_ballistic_arc(lua_State *l)
{
	int32_t argIdx = 1;
	auto &projPos = *Lua::CheckVector(l,argIdx++);
	auto projSpeed = Lua::CheckNumber(l,argIdx++);
	auto &tgt = *Lua::CheckVector(l,argIdx++);
	Vector3 *targetVel = nullptr;
	if(Lua::IsVector(l,argIdx) == true)
		targetVel = Lua::CheckVector(l,argIdx++);
	auto gravity = Lua::CheckNumber(l,argIdx++);
	std::array<Vector3,2> s;
	int32_t r = 0;
	if(targetVel == nullptr)
		r = umath::solve_ballistic_arc(projPos,projSpeed,tgt,gravity,s);
	else
		r = umath::solve_ballistic_arc(projPos,projSpeed,tgt,*targetVel,gravity,s);
	Lua::PushInt(l,r);
	for(auto i=decltype(r){0};i<r;++i)
		Lua::Push<Vector3>(l,s[i]);
	return r +1;
}
int Lua::math::solve_ballistic_arc_lateral(lua_State *l)
{
	int32_t argIdx = 1;
	auto &projPos = *Lua::CheckVector(l,argIdx++);
	auto lateralSpeed = Lua::CheckNumber(l,argIdx++);
	auto &tgt = *Lua::CheckVector(l,argIdx++);
	Vector3 fireVelocity;
	auto gravity = 0.f;
	Vector3 impactPoint;
	auto r = false;
	auto bHasTargetVel = Lua::IsVector(l,argIdx);
	if(bHasTargetVel == true)
	{
		auto &targetVel = *Lua::CheckVector(l,argIdx++);
		auto maxHeightOffset = Lua::CheckNumber(l,argIdx++);
		r = umath::solve_ballistic_arc_lateral(projPos,lateralSpeed,tgt,targetVel,maxHeightOffset,fireVelocity,gravity,impactPoint);
	}
	else
	{
		auto maxHeight = Lua::CheckNumber(l,argIdx++);
		r = umath::solve_ballistic_arc_lateral(projPos,lateralSpeed,tgt,maxHeight,fireVelocity,gravity);
	}
	if(r == false)
		return 0;
	Lua::Push<Vector3>(l,fireVelocity);
	Lua::PushNumber(l,gravity);
	if(bHasTargetVel == false)
		return 2;
	Lua::Push<Vector3>(l,impactPoint);
	return 3;
}
int Lua::math::abs_max(lua_State *l)
{
	auto v = 0.f;
	int32_t argIdx = 1;
	while(Lua::IsSet(l,argIdx))
	{
		auto n = Lua::CheckNumber(l,argIdx++);
		v = umath::abs_max(static_cast<float>(v),static_cast<float>(n));
	}
	Lua::PushNumber(l,v);
	return 1;
}
int Lua::math::ease_in(lua_State *l)
{
	auto argIdx = 1;
	auto t = Lua::CheckNumber(l,argIdx++);
	auto type = umath::EaseType::Linear;
	if(Lua::IsSet(l,argIdx))
		type = static_cast<umath::EaseType>(Lua::CheckInt(l,argIdx++));
	Lua::PushNumber(l,umath::ease_in(t,type));
	return 1;
}
int Lua::math::ease_out(lua_State *l)
{
	auto argIdx = 1;
	auto t = Lua::CheckNumber(l,argIdx++);
	auto type = umath::EaseType::Linear;
	if(Lua::IsSet(l,argIdx))
		type = static_cast<umath::EaseType>(Lua::CheckInt(l,argIdx++));
	Lua::PushNumber(l,umath::ease_out(t,type));
	return 1;
}
int Lua::math::ease_in_out(lua_State *l)
{
	auto argIdx = 1;
	auto t = Lua::CheckNumber(l,argIdx++);
	auto type = umath::EaseType::Linear;
	if(Lua::IsSet(l,argIdx))
		type = static_cast<umath::EaseType>(Lua::CheckInt(l,argIdx++));
	Lua::PushNumber(l,umath::ease_in_out(t,type));
	return 1;
}

int Lua::math::calc_horizontal_fov(lua_State *l)
{
	auto focalLengthInMM = Lua::CheckNumber(l,1);
	auto width = Lua::CheckNumber(l,2);
	auto height = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,::umath::calc_horizontal_fov(focalLengthInMM,width,height));
	return 1;
}
int Lua::math::calc_vertical_fov(lua_State *l)
{
	auto focalLengthInMM = Lua::CheckNumber(l,1);
	auto width = Lua::CheckNumber(l,2);
	auto height = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,::umath::calc_vertical_fov(focalLengthInMM,width,height));
	return 1;
}
int Lua::math::calc_diagonal_fov(lua_State *l)
{
	auto focalLengthInMM = Lua::CheckNumber(l,1);
	auto width = Lua::CheckNumber(l,2);
	auto height = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,::umath::calc_diagonal_fov(focalLengthInMM,width,height));
	return 1;
}

int Lua::math::horizontal_fov_to_vertical_fov(lua_State *l)
{
	auto hFov = Lua::CheckNumber(l,1);
	auto width = Lua::CheckNumber(l,2);
	auto height = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,::umath::horizontal_fov_to_vertical_fov(hFov,width,height));
	return 1;
}
int Lua::math::vertical_fov_to_horizontal_fov(lua_State *l)
{
	auto hFov = Lua::CheckNumber(l,1);
	auto width = Lua::CheckNumber(l,2);
	auto height = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,::umath::vertical_fov_to_horizontal_fov(hFov,width,height));
	return 1;
}
int Lua::math::diagonal_fov_to_vertical_fov(lua_State *l)
{
	auto diagonalFov = Lua::CheckNumber(l,1);
	auto aspectRatio = Lua::CheckNumber(l,2);
	Lua::PushNumber(l,::umath::diagonal_fov_to_vertical_fov(diagonalFov,aspectRatio));
	return 1;
}
int Lua::math::get_frustum_plane_center(lua_State *l)
{
	auto &pos = Lua::Check<Vector3>(l,1);
	auto &forward = Lua::Check<Vector3>(l,2);
	auto z = Lua::CheckNumber(l,3);
	Lua::Push<Vector3>(l,::umath::frustum::get_plane_center(pos,forward,z));
	return 1;
}
int Lua::math::get_frustum_plane_size(lua_State *l)
{
	auto fovRad = Lua::CheckNumber(l,1);
	auto aspectRatio = Lua::CheckNumber(l,2);
	auto z = Lua::CheckNumber(l,3);
	float w,h;
	::umath::frustum::get_plane_size(fovRad,z,aspectRatio,w,h);
	Lua::PushNumber(l,w);
	Lua::PushNumber(l,h);
	return 2;
}
int Lua::math::get_frustum_plane_boundaries(lua_State *l)
{
	auto &pos = Lua::Check<Vector3>(l,1);
	auto &forward = Lua::Check<Vector3>(l,2);
	auto &up = Lua::Check<Vector3>(l,3);
	auto fovRad = Lua::CheckNumber(l,4);
	auto aspectRatio = Lua::CheckNumber(l,5);
	auto z = Lua::CheckNumber(l,6);
	float w,h;
	auto boundaries = ::umath::frustum::get_plane_boundaries(pos,forward,up,fovRad,z,aspectRatio,&w,&h);
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(boundaries.size()){0u};i<boundaries.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::Push<Vector3>(l,boundaries.at(i));
		Lua::SetTableValue(l,t);
	}
	Lua::PushNumber(l,w);
	Lua::PushNumber(l,h);
	return 3;
}
int Lua::math::get_frustum_plane_point(lua_State *l)
{
	auto &pos = Lua::Check<Vector3>(l,1);
	auto &forward = Lua::Check<Vector3>(l,2);
	auto &right = Lua::Check<Vector3>(l,3);
	auto &up = Lua::Check<Vector3>(l,4);
	auto fovRad = Lua::CheckNumber(l,5);
	auto aspectRatio = Lua::CheckNumber(l,6);
	auto uv = Lua::Check<Vector2>(l,7);
	auto z = Lua::CheckNumber(l,8);
	Lua::Push<Vector3>(l,::umath::frustum::get_plane_point(pos,forward,right,up,fovRad,z,aspectRatio,uv));
	return 1;
}

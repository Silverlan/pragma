/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LVECTOR_H__
#define __LVECTOR_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <mathutil/glmutil.h>
#include <pragma/math/angle/wvangle.h>
#include <pragma/math/angle/wvquaternion.h>

struct Vertex;
struct VertexWeight;

namespace Lua
{
	namespace Vertex
	{
		DLLNETWORK ::Vertex Copy(::Vertex &v);
	};
	namespace VertexWeight
	{
		DLLNETWORK ::VertexWeight Copy(::VertexWeight &vw);
	};
	namespace Vectori
	{
		DLLNETWORK ::Vector3i Copy(::Vector3i &v);
	};
	namespace Vector2i
	{
		DLLNETWORK ::Vector2i Copy(::Vector2i &v);
	};
	namespace Vector4i
	{
		DLLNETWORK ::Vector4i Copy(::Vector4i &v);
	};

	namespace Vector
	{
		DLLNETWORK void Normalize(Vector3 &vec);
		DLLNETWORK void Rotate(lua_State *l,Vector3 &vec,const EulerAngles &ang);
		DLLNETWORK void Rotate(lua_State *l,Vector3 &vec,const Vector3 &normal,float angle);
		DLLNETWORK void Rotate(lua_State *l,Vector3 &vec,const Quat &orientation);
		DLLNETWORK void RotateAround(lua_State *l,Vector3 &vec,const Vector3 &origin,const EulerAngles &ang);
		DLLNETWORK void Copy(lua_State *l,const Vector3 &vec);
		DLLNETWORK void Lerp(lua_State *l,const Vector3 &vec,const Vector3 &vecB,float factor);
		DLLNETWORK void Set(lua_State *l,Vector3 &vec,const Vector3 &vecB);
		DLLNETWORK void ToMatrix(lua_State *l,const Vector3 &vec);
		DLLNETWORK void Set(lua_State *l,Vector3 &vec,float x,float y,float z);
		DLLNETWORK void SnapToGrid(lua_State *l,Vector3 &vec);
		DLLNETWORK void SnapToGrid(lua_State *l,Vector3 &vec,UInt32 gridSize);
	};
	namespace Vector2
	{
		DLLNETWORK ::Vector2 GetNormal(const ::Vector2 &vec);
		DLLNETWORK void Normalize(::Vector2 &vec);
		DLLNETWORK float Length(const ::Vector2 &vec);
		DLLNETWORK float LengthSqr(const ::Vector2 &vec);
		DLLNETWORK float Distance(const ::Vector2 &a,const ::Vector2 &b);
		DLLNETWORK float DistanceSqr(const ::Vector2 &a,const ::Vector2 &b);
		DLLNETWORK void Cross(lua_State *l,const ::Vector2 &a,const ::Vector2 &b);
		DLLNETWORK void DotProduct(lua_State *l,const ::Vector2 &a,const ::Vector2 &b);
		DLLNETWORK void Rotate(lua_State *l,::Vector2 &vec,const EulerAngles &ang);
		DLLNETWORK void RotateAround(lua_State *l,::Vector2 &vec,const ::Vector2 &origin,const EulerAngles &ang);
		DLLNETWORK void Copy(lua_State *l,const ::Vector2 &vec);
		DLLNETWORK void Lerp(lua_State *l,const ::Vector2 &vec,const ::Vector2 &vecB,float factor);
		DLLNETWORK void Set(lua_State *l,::Vector2 &vec,const ::Vector2 &vecB);
		DLLNETWORK void Set(lua_State *l,::Vector2 &vec,float x,float y);
		DLLNETWORK void Project(lua_State *l,const ::Vector2 &vec,const ::Vector2 &n);
	};
	namespace Vector4
	{
		DLLNETWORK ::Vector4 GetNormal(const ::Vector4 &vec);
		DLLNETWORK void Normalize(::Vector4 &vec);
		DLLNETWORK float Length(const ::Vector4 &vec);
		DLLNETWORK float LengthSqr(const ::Vector4 &vec);
		DLLNETWORK float Distance(const ::Vector4 &a,const ::Vector4 &b);
		DLLNETWORK float DistanceSqr(const ::Vector4 &a,const ::Vector4 &b);
		DLLNETWORK void DotProduct(lua_State *l,const ::Vector4 &a,const ::Vector4 &b);
		DLLNETWORK void Copy(lua_State *l,const ::Vector4 &vec);
		DLLNETWORK void Lerp(lua_State *l,const ::Vector4 &vec,const ::Vector4 &vecB,float factor);
		DLLNETWORK void Set(lua_State *l,::Vector4 &vec,const ::Vector4 &vecB);
		DLLNETWORK void Set(lua_State *l,::Vector4 &vec,float x,float y,float z,float w);
		DLLNETWORK void Project(lua_State *l,const ::Vector4 &vec,const ::Vector4 &n);
	};
	namespace vector
	{
		DLLNETWORK void to_min_max(::Vector2 &inOutA,::Vector2 &inOutB);
		DLLNETWORK void to_min_max(::Vector3 &inOutA,::Vector3 &inOutB);
		DLLNETWORK void to_min_max(::Vector4 &inOutA,::Vector4 &inOutB);
		DLLNETWORK void to_min_max(::Vector2 &inOutA,::Vector2 &inOutB,const ::Vector2 &c);
		DLLNETWORK void to_min_max(::Vector3 &inOutA,::Vector3 &inOutB,const ::Vector3 &c);
		DLLNETWORK void to_min_max(::Vector4 &inOutA,::Vector4 &inOutB,const ::Vector4 &c);
		DLLNETWORK void get_min_max(lua_State *l,luabind::table<> t,::Vector2 &outMin,::Vector2 &outMax);
		DLLNETWORK void get_min_max(lua_State *l,luabind::table<> t,::Vector3 &outMin,::Vector3 &outMax);
		DLLNETWORK void get_min_max(lua_State *l,luabind::table<> t,::Vector4 &outMin,::Vector4 &outMax);
		DLLNETWORK ::Vector2 random_2d();
		DLLNETWORK ::Vector3 calc_average(luabind::table<> points);
		DLLNETWORK void calc_best_fitting_plane(luabind::table<> points,float ang,Vector3 &outNormal,double &outDistance);
	};
};

#endif

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
		DLLNETWORK void Copy(lua_State *l,::Vertex &v);
	};
	namespace VertexWeight
	{
		DLLNETWORK void Copy(lua_State *l,::VertexWeight &vw);
	};
	namespace Vectori
	{
		DLLNETWORK void Copy(lua_State *l,::Vector3i &v);
	};
	namespace Vector2i
	{
		DLLNETWORK void Copy(lua_State *l,::Vector2i &v);
	};
	namespace Vector4i
	{
		DLLNETWORK void Copy(lua_State *l,::Vector4i &v);
	};

	namespace Vector
	{
		DLLNETWORK Vector3 GetNormal(const Vector3 &vec);
		DLLNETWORK void Normalize(Vector3 &vec);
		DLLNETWORK EulerAngles Angle(const Vector3 &vec);
		DLLNETWORK float Length(const Vector3 &vec);
		DLLNETWORK float LengthSqr(const Vector3 &vec);
		DLLNETWORK float Distance(const Vector3 &a,const Vector3 &b);
		DLLNETWORK float DistanceSqr(const Vector3 &a,const Vector3 &b);
		DLLNETWORK float PlanarDistance(const Vector3 &a,const Vector3 &b,const Vector3 &n);
		DLLNETWORK float PlanarDistanceSqr(const Vector3 &a,const Vector3 &b,const Vector3 &n);
		DLLNETWORK void Cross(lua_State *l,const Vector3 &a,const Vector3 &b);
		DLLNETWORK void DotProduct(lua_State *l,const Vector3 &a,const Vector3 &b);
		DLLNETWORK void GetRotation(lua_State *l,const Vector3 &va,const Vector3 &vb);
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
		DLLNETWORK void Project(lua_State *l,const Vector3 &vec,const Vector3 &n);
		DLLNETWORK void ProjectToPlane(lua_State *l,const Vector3 &p,const Vector3 &n,float d);
		DLLNETWORK void GetPerpendicular(lua_State *l,const Vector3 &vec);
		DLLNETWORK void OuterProduct(lua_State *l,const Vector3 &v0,const Vector3 &v1);
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
		DLLNETWORK int to_min_max(lua_State *l);
		DLLNETWORK int get_min_max(lua_State *l);
		DLLNETWORK int random(lua_State *l);
		DLLNETWORK int random_2d(lua_State *l);
		DLLNETWORK int create_from_string(lua_State *l);
		DLLNETWORK int calc_average(lua_State *l);
		DLLNETWORK int calc_best_fitting_plane(lua_State *l);
		DLLNETWORK int angular_velocity_to_linear(lua_State *l);
	};
};

#endif

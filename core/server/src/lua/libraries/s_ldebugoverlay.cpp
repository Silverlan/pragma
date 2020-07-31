/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/lua/libraries/s_ldebugoverlay.h"
#include "pragma/debug/debugoverlay.h"
#include "pragma/lua/classes/ldef_color.h"
#include "pragma/lua/classes/ldef_angle.h"
#include <pragma/lua/classes/ldef_vector.h>
#include "pragma/lua/classes/ldef_plane.h"
#include "luasystem.h"

int Lua::DebugRenderer::Server::DrawPoint(lua_State *l)
{
	auto *pos = Lua::CheckVector(l,1);
	auto *col = Lua::CheckColor(l,2);
	auto duration = Lua::CheckNumber(l,3);
	SDebugRenderer::DrawPoint(*pos,*col,static_cast<float>(duration));
	return 0;
}
int Lua::DebugRenderer::Server::DrawLine(lua_State *l)
{
	auto *start = Lua::CheckVector(l,1);
	auto *end = Lua::CheckVector(l,2);
	auto *col = Lua::CheckColor(l,3);
	auto duration = Lua::CheckNumber(l,4);
	SDebugRenderer::DrawLine(*start,*end,*col,static_cast<float>(duration));
	return 0;
}
int Lua::DebugRenderer::Server::DrawBox(lua_State *l)
{
	int32_t arg = 1;
	auto *center = Lua::CheckVector(l,arg++);
	auto *start = Lua::CheckVector(l,arg++);
	Vector3 *end = nullptr;
	if(Lua::IsVector(l,arg))
		end = Lua::CheckVector(l,arg++);
	EulerAngles ang{0.f,0.f,0.f};
	if(Lua::IsEulerAngles(l,arg))
	{
		auto *eulerAngles = Lua::CheckEulerAngles(l,arg++);
		ang = *eulerAngles;
	}
	auto *col = Lua::CheckColor(l,arg++);
	Color *colOutline = nullptr;
	if(Lua::IsColor(l,arg) == true)
		colOutline = Lua::CheckColor(l,arg++);
	auto duration = Lua::CheckNumber(l,arg++);
	if(end != nullptr)
	{
		if(colOutline != nullptr)
			::SDebugRenderer::DrawBox(*center,*start,*end,ang,*col,*colOutline,static_cast<float>(duration));
		else
			::SDebugRenderer::DrawBox(*center,*start,*end,ang,*col,static_cast<float>(duration));
	}
	else
	{
		if(colOutline != nullptr)
			::SDebugRenderer::DrawBox(*center,*start,ang,*col,*colOutline,static_cast<float>(duration));
		else
			::SDebugRenderer::DrawBox(*center,*start,ang,*col,static_cast<float>(duration));
	}
	return 0;
}
int Lua::DebugRenderer::Server::DrawSphere(lua_State *l)
{
	auto *origin = Lua::CheckVector(l,1);
	auto radius = Lua::CheckNumber(l,2);
	auto *col = Lua::CheckColor(l,3);
	auto duration = 0.f;
	std::unique_ptr<Color> outlineColor = nullptr;
	auto idx = 4;
	if(Lua::IsNumber(l,idx))
		duration = static_cast<float>(Lua::CheckNumber(l,idx++));
	else
	{
		auto *colOutline = Lua::CheckColor(l,idx++);
		outlineColor = std::make_unique<Color>(*colOutline);
		duration = static_cast<float>(Lua::CheckNumber(l,idx++));
	}
	uint32_t recursionLevel = 1;
	if(Lua::IsSet(l,idx))
		recursionLevel = static_cast<uint32_t>(Lua::CheckInt(l,idx++));
	if(outlineColor == nullptr)
		SDebugRenderer::DrawSphere(*origin,static_cast<float>(radius),*col,duration,recursionLevel);
	else
		SDebugRenderer::DrawSphere(*origin,static_cast<float>(radius),*col,*outlineColor,duration,recursionLevel);
	return 0;
}
int Lua::DebugRenderer::Server::DrawCone(lua_State *l)
{
	auto *origin = Lua::CheckVector(l,1);
	auto *dir = Lua::CheckVector(l,2);
	auto dist = Lua::CheckNumber(l,3);
	auto angle = Lua::CheckNumber(l,4);

	auto *col = Lua::CheckColor(l,5);
	auto duration = 0.f;
	std::unique_ptr<Color> outlineColor = nullptr;
	auto idx = 6;
	if(Lua::IsSet(l,idx))
	{
		if(Lua::IsNumber(l,idx))
			duration = static_cast<float>(Lua::CheckNumber(l,idx++));
		else
		{
			auto *colOutline = Lua::CheckColor(l,idx++);
			outlineColor = std::make_unique<Color>(*colOutline);
			if(Lua::IsSet(l,idx) && Lua::IsNumber(l,idx))
				duration = static_cast<float>(Lua::CheckNumber(l,idx++));
		}
	}
	uint32_t segmentCount = 12;
	if(Lua::IsSet(l,idx))
		segmentCount = static_cast<uint32_t>(Lua::CheckInt(l,idx++));
	if(outlineColor == nullptr)
		SDebugRenderer::DrawCone(*origin,*dir,static_cast<float>(dist),static_cast<float>(angle),*col,duration,segmentCount);
	else
		SDebugRenderer::DrawCone(*origin,*dir,static_cast<float>(dist),static_cast<float>(angle),*col,*outlineColor,duration,segmentCount);
	return 0;
}
int Lua::DebugRenderer::Server::DrawTruncatedCone(lua_State *l)
{
	auto *origin = Lua::CheckVector(l,1);
	auto startRadius = Lua::CheckNumber(l,2);
	auto *dir = Lua::CheckVector(l,3);
	auto dist = Lua::CheckNumber(l,4);
	auto endRadius = Lua::CheckNumber(l,5);

	auto *col = Lua::CheckColor(l,6);
	auto duration = 0.f;
	std::unique_ptr<Color> outlineColor = nullptr;
	auto idx = 7;
	if(Lua::IsSet(l,idx))
	{
		if(Lua::IsNumber(l,idx))
			duration = static_cast<float>(Lua::CheckNumber(l,idx++));
		else
		{
			auto *colOutline = Lua::CheckColor(l,idx++);
			outlineColor = std::make_unique<Color>(*colOutline);
			if(Lua::IsSet(l,idx) && Lua::IsNumber(l,idx))
				duration = static_cast<float>(Lua::CheckNumber(l,idx++));
		}
	}
	uint32_t segmentCount = 12;
	if(Lua::IsSet(l,idx))
		segmentCount = static_cast<uint32_t>(Lua::CheckInt(l,idx++));
	if(outlineColor == nullptr)
		SDebugRenderer::DrawTruncatedCone(*origin,static_cast<float>(startRadius),*dir,static_cast<float>(dist),static_cast<float>(endRadius),*col,duration,segmentCount);
	else
		SDebugRenderer::DrawTruncatedCone(*origin,static_cast<float>(startRadius),*dir,static_cast<float>(dist),static_cast<float>(endRadius),*col,*outlineColor,duration,segmentCount);
	return 0;
}
int Lua::DebugRenderer::Server::DrawCylinder(lua_State *l)
{
	auto *origin = Lua::CheckVector(l,1);
	auto *dir = Lua::CheckVector(l,2);
	auto dist = Lua::CheckNumber(l,3);
	auto radius = Lua::CheckNumber(l,4);

	auto *col = Lua::CheckColor(l,5);
	auto duration = 0.f;
	std::unique_ptr<Color> outlineColor = nullptr;
	auto idx = 6;
	if(Lua::IsSet(l,idx))
	{
		if(Lua::IsNumber(l,idx))
			duration = static_cast<float>(Lua::CheckNumber(l,idx++));
		else
		{
			auto *colOutline = Lua::CheckColor(l,idx++);
			outlineColor = std::make_unique<Color>(*colOutline);
			if(Lua::IsSet(l,idx) && Lua::IsNumber(l,idx))
				duration = static_cast<float>(Lua::CheckNumber(l,idx++));
		}
	}
	uint32_t segmentCount = 12;
	if(Lua::IsSet(l,idx))
		segmentCount = static_cast<uint32_t>(Lua::CheckInt(l,idx++));
	if(outlineColor == nullptr)
		SDebugRenderer::DrawCylinder(*origin,*dir,static_cast<float>(dist),static_cast<float>(radius),*col,static_cast<float>(duration),segmentCount);
	else
		SDebugRenderer::DrawCylinder(*origin,*dir,static_cast<float>(dist),static_cast<float>(radius),*col,*outlineColor,static_cast<float>(duration),segmentCount);
	return 0;
}
int Lua::DebugRenderer::Server::DrawAxis(lua_State *l)
{
	auto duration = 0.f;
	std::optional<Vector3> origin {};
	std::optional<EulerAngles> ang {};
	if(Lua::IsType<umath::Transform>(l,1))
	{
		auto &pose = Lua::Check<umath::Transform>(l,1);
		origin = pose.GetOrigin();
		ang = EulerAngles{pose.GetRotation()};

		if(Lua::IsSet(l,2))
			duration = static_cast<float>(Lua::CheckNumber(l,2));
	}
	else
	{
		origin = *Lua::CheckVector(l,1);
		if(Lua::IsEulerAngles(l,2))
		{
			ang = *Lua::CheckEulerAngles(l,2);
			duration = static_cast<float>(Lua::CheckNumber(l,3));
			SDebugRenderer::DrawAxis(*origin,*ang,duration);
			return 0;
		}
		else
			duration = static_cast<float>(Lua::CheckNumber(l,2));
	}
	SDebugRenderer::DrawAxis(*origin,duration);
	return 0;
}
int Lua::DebugRenderer::Server::DrawText(lua_State *l)
{
	int32_t arg = 1;
	auto *text = Lua::CheckString(l,arg++);
	auto &origin = *Lua::CheckVector(l,arg++);
	Vector2 *size = nullptr;
	auto scale = 1.f;
	if(Lua::IsVector2(l,arg))
		size = Lua::CheckVector2(l,arg++);
	else
		scale = static_cast<float>(Lua::CheckNumber(l,arg++));
	Color *col = nullptr;
	if(Lua::IsColor(l,arg))
		col = Lua::CheckColor(l,arg++);
	auto duration = 0.f;
	if(Lua::IsSet(l,arg))
		duration = static_cast<float>(Lua::CheckNumber(l,arg++));
	if(col != nullptr)
	{
		if(size != nullptr)
			::SDebugRenderer::DrawText(text,origin,*size,*col,static_cast<float>(duration));
		else
			::SDebugRenderer::DrawText(text,origin,scale,*col,static_cast<float>(duration));
	}
	else
	{
		if(size != nullptr)
			::SDebugRenderer::DrawText(text,origin,*size,static_cast<float>(duration));
		else
			::SDebugRenderer::DrawText(text,origin,scale,static_cast<float>(duration));
	}
	return 0;
}
static void get_path(lua_State *l,std::vector<Vector3> &path)
{
	Lua::CheckTable(l,1);

	auto numPath = Lua::GetObjectLength(l,1);
	path.reserve(numPath);
	int32_t t = 1;
	for(auto i=decltype(numPath){0};i<numPath;++i)
	{
		Lua::PushInt(l,i +1); /* 1 */
		Lua::GetTableValue(l,t); /* 1 */
		auto &v = *Lua::CheckVector(l,-1);
		path.push_back(v);

		Lua::Pop(l,1); /* 0 */
	}
}
int Lua::DebugRenderer::Server::DrawPath(lua_State *l)
{
	std::vector<Vector3> path;
	get_path(l,path);

	auto &col = *Lua::CheckColor(l,2);
	auto duration = Lua::CheckNumber(l,3);

	::SDebugRenderer::DrawPath(path,col,static_cast<float>(duration));
	return 0;
}
int Lua::DebugRenderer::Server::DrawSpline(lua_State *l)
{
	std::vector<Vector3> path;
	get_path(l,path);

	auto &col = *Lua::CheckColor(l,2);
	auto duration = 0.f;

	auto numSegments = Lua::CheckInt(l,3);
	auto curvature = 1.f;
	if(Lua::IsSet(l,5))
	{
		curvature = static_cast<float>(Lua::CheckNumber(l,4));
		duration = static_cast<float>(Lua::CheckNumber(l,5));
	}
	else
		duration = static_cast<float>(Lua::CheckNumber(l,4));

	::SDebugRenderer::DrawSpline(path,col,static_cast<uint32_t>(numSegments),curvature,duration);
	return 0;
}
int Lua::DebugRenderer::Server::DrawPlane(lua_State *l)
{
	auto &col = *Lua::CheckColor(l,2);
	auto duration = 0.f;
	if(Lua::IsSet(l,3))
		duration = static_cast<float>(Lua::CheckNumber(l,3));
	if(Lua::IsPlane(l,1) == true)
	{
		auto &plane = *Lua::CheckPlane(l,1);
		::SDebugRenderer::DrawPlane(plane,col,duration);
		return 0;
	}
	auto &n = *Lua::CheckVector(l,1);
	auto d = Lua::CheckNumber(l,2);

	::SDebugRenderer::DrawPlane(n,static_cast<float>(d),col,duration);
	return 0;
}

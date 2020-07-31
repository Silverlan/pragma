/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/libraries/c_ldebugoverlay.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/classes/ldef_color.h"
#include <pragma/lua/classes/ldef_vector.h>
#include "pragma/lua/classes/ldef_plane.h"
#include "luasystem.h"

int Lua::DebugRenderer::Client::DrawPoints(lua_State *l)
{
	Lua::CheckTable(l,1);
	auto *col = Lua::CheckColor(l,2);
	auto duration = 0.f;
	if(Lua::IsSet(l,3))
		duration = static_cast<float>(Lua::CheckNumber(l,3));

	std::vector<Vector3> points;
	Lua::PushValue(l,1); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushNil(l); /* 2 */
	while(Lua::GetNextPair(l,t) != 0) /* 3 */
	{
		auto *v = Lua::CheckVector(l,-1); /* 3 */
		points.push_back(*v);
		Lua::Pop(l,1); /* 2 */
	} /* 1 */
	Lua::Pop(l,1); /* 0 */

	auto r = ::DebugRenderer::DrawPoints(points,*col,duration);
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}

int Lua::DebugRenderer::Client::DrawLines(lua_State *l)
{
	Lua::CheckTable(l,1);
	auto *col = Lua::CheckColor(l,2);
	auto duration = 0.f;
	if(Lua::IsSet(l,3))
		duration = static_cast<float>(Lua::CheckNumber(l,3));

	std::vector<Vector3> lines;
	Lua::PushValue(l,1); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushNil(l); /* 2 */
	while(Lua::GetNextPair(l,t) != 0) /* 3 */
	{
		auto *v = Lua::CheckVector(l,-1); /* 3 */
		lines.push_back(*v);
		Lua::Pop(l,1); /* 2 */
	} /* 1 */
	Lua::Pop(l,1); /* 0 */

	auto r = ::DebugRenderer::DrawLines(lines,*col,duration);
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}

int Lua::DebugRenderer::Client::DrawPoint(lua_State *l)
{
	auto *pos = Lua::CheckVector(l,1);
	auto *col = Lua::CheckColor(l,2);
	auto duration = 0.f;
	if(Lua::IsSet(l,3))
		duration = static_cast<float>(Lua::CheckNumber(l,3));
	auto r = ::DebugRenderer::DrawPoint(*pos,*col,duration);
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawLine(lua_State *l)
{
	auto *start = Lua::CheckVector(l,1);
	auto *end = Lua::CheckVector(l,2);
	auto *col = Lua::CheckColor(l,3);
	auto duration = 0.f;
	if(Lua::IsSet(l,4))
		duration = static_cast<float>(Lua::CheckNumber(l,4));
	auto r = ::DebugRenderer::DrawLine(*start,*end,*col,duration);
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawBox(lua_State *l)
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
	auto duration = 0.f;
	if(Lua::IsSet(l,arg))
		duration = static_cast<float>(Lua::CheckNumber(l,arg++));
	std::shared_ptr<::DebugRenderer::BaseObject> r = nullptr;
	if(end != nullptr)
	{
		if(colOutline != nullptr)
			r = ::DebugRenderer::DrawBox(*center,*start,*end,ang,*col,*colOutline,duration);
		else
			r = ::DebugRenderer::DrawBox(*center,*start,*end,ang,*col,duration);
	}
	else
	{
		if(colOutline != nullptr)
			r = ::DebugRenderer::DrawBox(*center,*start,ang,*col,*colOutline,duration);
		else
			r = ::DebugRenderer::DrawBox(*center,*start,ang,*col,duration);
	}
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawMeshes(lua_State *l)
{
	std::vector<Vector3> verts;

	Lua::CheckTable(l,1);
	auto *col = Lua::CheckColor(l,2);
	auto duration = 0.f;
	std::unique_ptr<Color> outlineColor = nullptr;
	if(Lua::IsSet(l,3))
	{
		if(Lua::IsNumber(l,3))
			duration = static_cast<float>(Lua::CheckNumber(l,3));
		else
		{
			auto *colOutline = Lua::CheckColor(l,3);
			outlineColor = std::make_unique<Color>(*colOutline);
			if(Lua::IsSet(l,4))
				duration = static_cast<float>(Lua::CheckNumber(l,4));
		}
	}
	Lua::PushValue(l,1); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushNil(l); /* 2 */
	while(Lua::GetNextPair(l,t) != 0) /* 3 */
	{
		auto *v = Lua::CheckVector(l,-1); /* 3 */
		verts.push_back(*v);
		Lua::Pop(l,1); /* 2 */
	} /* 1 */
	Lua::Pop(l,1); /* 0 */

	std::shared_ptr<::DebugRenderer::BaseObject> r = nullptr;
	if(outlineColor == nullptr)
		r = ::DebugRenderer::DrawMesh(verts,*col,duration);
	else
		r = ::DebugRenderer::DrawMesh(verts,*col,*outlineColor,duration);
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawTruncatedCone(lua_State *l)
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
		segmentCount = Lua::CheckInt(l,idx++);
	std::shared_ptr<::DebugRenderer::BaseObject> r = nullptr;
	if(outlineColor == nullptr)
		r = ::DebugRenderer::DrawTruncatedCone(*origin,startRadius,*dir,dist,endRadius,*col,duration,segmentCount);
	else
		r = ::DebugRenderer::DrawTruncatedCone(*origin,startRadius,*dir,dist,endRadius,*col,*outlineColor,duration,segmentCount);
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawCylinder(lua_State *l)
{
	auto *origin = Lua::CheckVector(l,1);
	auto radius = Lua::CheckNumber(l,2);
	auto *dir = Lua::CheckVector(l,3);
	auto dist = Lua::CheckNumber(l,4);

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
		segmentCount = Lua::CheckInt(l,idx++);
	std::shared_ptr<::DebugRenderer::BaseObject> r = nullptr;
	if(outlineColor == nullptr)
		r = ::DebugRenderer::DrawCylinder(*origin,*dir,dist,radius,*col,duration,segmentCount);
	else
		r = ::DebugRenderer::DrawCylinder(*origin,*dir,dist,radius,*col,*outlineColor,duration,segmentCount);
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawCone(lua_State *l)
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
		segmentCount = Lua::CheckInt(l,idx++);
	std::shared_ptr<::DebugRenderer::BaseObject> r = nullptr;
	if(outlineColor == nullptr)
		r = ::DebugRenderer::DrawCone(*origin,*dir,dist,angle,*col,duration,segmentCount);
	else
		r = ::DebugRenderer::DrawCone(*origin,*dir,dist,angle,*col,*outlineColor,duration,segmentCount);
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawSphere(lua_State *l)
{
	auto *origin = Lua::CheckVector(l,1);
	auto radius = Lua::CheckNumber(l,2);
	auto *col = Lua::CheckColor(l,3);
	auto duration = 0.f;
	std::unique_ptr<Color> outlineColor = nullptr;
	auto idx = 4;
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
	uint32_t recursionLevel = 1;
	if(Lua::IsSet(l,idx))
		recursionLevel = Lua::CheckInt(l,idx++);
	std::shared_ptr<::DebugRenderer::BaseObject> r = nullptr;
	if(outlineColor == nullptr)
		r = ::DebugRenderer::DrawSphere(*origin,radius,*col,duration,recursionLevel);
	else
		r = ::DebugRenderer::DrawSphere(*origin,radius,*col,*outlineColor,duration,recursionLevel);
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawAxis(lua_State *l)
{
	if(Lua::IsType<umath::Transform>(l,1))
	{
		auto &pose = Lua::Check<umath::Transform>(l,1);
		auto duration = 0.f;
		if(Lua::IsSet(l,2))
			duration = static_cast<float>(Lua::CheckNumber(l,2));
		auto r = ::DebugRenderer::DrawAxis(pose.GetOrigin(),EulerAngles{pose.GetRotation()},duration);
		auto t = Lua::CreateTable(l);
		for(auto i=decltype(r.size()){0};i<r.size();++i)
		{
			Lua::PushInt(l,i +1);
			Lua::Push<decltype(r[i])>(l,r[i]);
			Lua::SetTableValue(l,t);
		}
		return 1;
	}
	auto *origin = Lua::CheckVector(l,1);
	std::array<std::shared_ptr<::DebugRenderer::BaseObject>,3> r{nullptr};
	auto duration = 0.f;
	if(Lua::IsSet(l,2))
	{
		if(Lua::IsEulerAngles(l,2))
		{
			auto *ang = Lua::CheckEulerAngles(l,2);
			if(Lua::IsSet(l,3))
				duration = static_cast<float>(Lua::CheckNumber(l,3));
			r = ::DebugRenderer::DrawAxis(*origin,*ang,duration);
		}
		else if(Lua::IsVector(l,2))
		{
			auto *x = Lua::CheckVector(l,2);
			auto *y = Lua::CheckVector(l,3);
			auto *z = Lua::CheckVector(l,4);
			if(Lua::IsSet(l,5))
				duration = static_cast<float>(Lua::CheckNumber(l,5));
			r = ::DebugRenderer::DrawAxis(*origin,*x,*y,*z,duration);
		}
		else
			duration = Lua::CheckNumber(l,2);
	}
	if(r[0] == nullptr)
		r = ::DebugRenderer::DrawAxis(*origin,duration);
	if(duration > 0.f)
		return 0;
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(r.size()){0};i<r.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::Push<decltype(r[i])>(l,r[i]);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
int Lua::DebugRenderer::Client::DrawText(lua_State *l)
{
	int32_t arg = 1;
	auto *text = Lua::CheckString(l,arg++);
	auto &origin = *Lua::CheckVector(l,arg++);
	Vector2 *size = nullptr;
	auto scale = 1.f;
	if(Lua::IsVector2(l,arg))
		size = Lua::CheckVector2(l,arg++);
	else
		scale = Lua::CheckNumber(l,arg++);
	Color *col = nullptr;
	if(Lua::IsColor(l,arg))
		col = Lua::CheckColor(l,arg++);
	auto duration = 0.f;
	if(Lua::IsSet(l,arg))
		duration = static_cast<float>(Lua::CheckNumber(l,arg++));
	std::shared_ptr<::DebugRenderer::BaseObject> r = nullptr;
	if(col != nullptr)
	{
		if(size != nullptr)
			r = ::DebugRenderer::DrawText(text,origin,*size,*col,duration);
		else
			r = ::DebugRenderer::DrawText(text,origin,scale,*col,duration);
	}
	else
	{
		if(size != nullptr)
			r = ::DebugRenderer::DrawText(text,origin,*size,duration);
		else
			r = ::DebugRenderer::DrawText(text,origin,scale,duration);
	}
	if(duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
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
int Lua::DebugRenderer::Client::DrawPath(lua_State *l)
{
	std::vector<Vector3> path;
	get_path(l,path);

	auto &col = *Lua::CheckColor(l,2);
	auto duration = 0.f;
	if(Lua::IsSet(l,3))
		duration = static_cast<float>(Lua::CheckNumber(l,3));

	auto r = ::DebugRenderer::DrawPath(path,col,duration);
	if(r == nullptr || duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawSpline(lua_State *l)
{
	std::vector<Vector3> path;
	get_path(l,path);

	auto &col = *Lua::CheckColor(l,2);
	auto duration = 0.f;

	auto numSegments = Lua::CheckInt(l,3);
	auto curvature = 1.f;
	if(Lua::IsSet(l,4))
	{
		if(Lua::IsSet(l,5))
		{
			curvature = Lua::CheckNumber(l,4);
			duration = static_cast<float>(Lua::CheckNumber(l,5));
		}
		else
			duration = static_cast<float>(Lua::CheckNumber(l,4));
	}

	auto r = ::DebugRenderer::DrawSpline(path,col,numSegments,curvature,duration);
	if(r == nullptr || duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawPlane(lua_State *l)
{
	if(Lua::IsPlane(l,1) == true)
	{
		auto &plane = *Lua::CheckPlane(l,1);
		auto &col = *Lua::CheckColor(l,2);
		auto duration = 0.f;
		if(Lua::IsSet(l,3))
			duration = static_cast<float>(Lua::CheckNumber(l,3));
		auto r = ::DebugRenderer::DrawPlane(plane,col,duration);
		if(r == nullptr || duration > 0.f)
			return 0;
		Lua::Push<decltype(r)>(l,r);
		return 1;
	}
	auto &n = *Lua::CheckVector(l,1);
	auto d = Lua::CheckNumber(l,2);
	auto &col = *Lua::CheckColor(l,3);
	auto duration = 0.f;
	if(Lua::IsSet(l,4))
		duration = static_cast<float>(Lua::CheckNumber(l,4));

	auto r = ::DebugRenderer::DrawPlane(n,d,col,duration);
	if(r == nullptr || duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
int Lua::DebugRenderer::Client::DrawFrustum(lua_State *l)
{
	std::vector<Vector3> points {};
	points.reserve(8u);
	if(Lua::IsCCamera(l,1))
	{
		auto &cam = *Lua::CheckCCamera(l,1);
		cam->GetFrustumPoints(points);
	}
	else
	{
		int32_t t = 1;
		Lua::CheckTable(l,t);
		for(auto i=uint32_t{0};i<8;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,t);
			auto &v = *Lua::CheckVector(l,-1);
			points.push_back(v);
			Lua::Pop(l,1);
		}
	}
	auto duration = 0.f;
	if(Lua::IsSet(l,2))
		duration = Lua::CheckNumber(l,2);

	auto r = ::DebugRenderer::DrawFrustum(points,duration);
	if(r == nullptr || duration > 0.f)
		return 0;
	Lua::Push<decltype(r)>(l,r);
	return 1;
}
void Lua::DebugRenderer::Client::Object::Remove(lua_State *l,::DebugRenderer::BaseObject &o)
{
	o.Remove();
}
void Lua::DebugRenderer::Client::Object::IsValid(lua_State *l,::DebugRenderer::BaseObject &o)
{
	Lua::PushBool(l,o.IsValid());
}
void Lua::DebugRenderer::Client::Object::SetPos(lua_State *l,::DebugRenderer::BaseObject &o,const Vector3 &pos)
{
	o.SetPos(pos);
}
void Lua::DebugRenderer::Client::Object::GetPos(lua_State *l,::DebugRenderer::BaseObject &o)
{
	Lua::Push<Vector3>(l,o.GetPos());
}
void Lua::DebugRenderer::Client::Object::SetRotation(lua_State *l,::DebugRenderer::BaseObject &o,const Quat &rot)
{
	o.SetRotation(rot);
}
void Lua::DebugRenderer::Client::Object::GetRotation(lua_State *l,::DebugRenderer::BaseObject &o)
{
	Lua::Push<Quat>(l,o.GetRotation());
}
void Lua::DebugRenderer::Client::Object::SetAngles(lua_State *l,::DebugRenderer::BaseObject &o,const EulerAngles &ang)
{
	o.SetAngles(ang);
}
void Lua::DebugRenderer::Client::Object::GetAngles(lua_State *l,::DebugRenderer::BaseObject &o)
{
	Lua::Push<EulerAngles>(l,o.GetAngles());
}
void Lua::DebugRenderer::Client::Object::IsVisible(lua_State *l,::DebugRenderer::BaseObject &o) {Lua::PushBool(l,o.IsVisible());}
void Lua::DebugRenderer::Client::Object::SetVisible(lua_State *l,::DebugRenderer::BaseObject &o,bool b) {o.SetVisible(b);}

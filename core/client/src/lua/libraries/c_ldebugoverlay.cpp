/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/libraries/c_ldebugoverlay.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/classes/ldef_color.h"
#include <pragma/lua/classes/ldef_vector.h>
#include "pragma/lua/classes/ldef_plane.h"
#include "luasystem.h"

static std::shared_ptr<DebugRenderer::BaseObject> get_dbg_object(const std::shared_ptr<DebugRenderer::BaseObject> &obj,float duration)
{
	return (duration > 0.f) ? nullptr : obj;
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPoints(const std::vector<Vector3> &points,const Color &col,float duration)
{
	return get_dbg_object(::DebugRenderer::DrawPoints(points,col,duration),duration);
}

std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawLines(const std::vector<Vector3> &linePoints,const Color &col,float duration)
{
	return get_dbg_object(::DebugRenderer::DrawLines(linePoints,col,duration),duration);
}

std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawLines(const std::vector<Vector3> &linePoints,const Color &col,float duration)
{
	return get_dbg_object(::DebugRenderer::DrawLines(linePoints,col,duration),duration);
}

std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPoint(const Vector3 &pos,const Color &color,float duration)
{
	return get_dbg_object(::DebugRenderer::DrawPoint(pos,color,duration),duration);
}

std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawLine(const Vector3 &start,const Vector3 &end,const Color &color,float duration)
{
	return get_dbg_object(::DebugRenderer::DrawLine(start,end,color,duration),duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawBox(
	const Vector3 &center,const Vector3 &start,const Vector3 &end,const Color &color,const std::optional<Color> &colOutline,float duration,const EulerAngles &angles
)
{
	return get_dbg_object(::DebugRenderer::DrawBox(center,start,end,angles,color,*colOutline,duration),duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawBox(
	const Vector3 &center,const Vector3 &start,const Color &color,const std::optional<Color> &colOutline,float duration,const EulerAngles &angles
)
{
	return get_dbg_object(::DebugRenderer::DrawBox(center,start,angles,color,*colOutline,duration),duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawMeshes(
	const std::vector<Vector3> &verts,const Color &color,const std::optional<Color> &outlineColor,float duration
)
{
	if(outlineColor.has_value())
		return get_dbg_object(::DebugRenderer::DrawMesh(verts,color,*outlineColor,duration),duration);
	return get_dbg_object(::DebugRenderer::DrawMesh(verts,color,duration),duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawTruncatedCone(
	const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,const Color &col,const std::optional<Color> &outlineColor,float duration,uint32_t segmentCount
)
{
	if(outlineColor.has_value())
		return get_dbg_object(::DebugRenderer::DrawTruncatedCone(origin,startRadius,dir,dist,endRadius,col,*outlineColor,duration,segmentCount),duration);
	return get_dbg_object(::DebugRenderer::DrawTruncatedCone(origin,startRadius,dir,dist,endRadius,col,duration,segmentCount),duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawCylinder(
	const Vector3 &origin,float radius,const Vector3 &dir,float dist,const Color &color,const std::optional<Color> &outlineColor,float duration,uint32_t segmentCount
)
{
	if(outlineColor.has_value())
		return get_dbg_object(::DebugRenderer::DrawCylinder(origin,dir,dist,radius,color,*outlineColor,duration,segmentCount),duration);
	return get_dbg_object(::DebugRenderer::DrawCylinder(origin,dir,dist,radius,color,duration,segmentCount),duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawCone(
	const Vector3 &origin,const Vector3 &dir,float dist,float angle,const Color &col,const std::optional<Color> &outlineColor,float duration,uint32_t segmentCount
)
{
	if(outlineColor.has_value())
		return get_dbg_object(::DebugRenderer::DrawCone(origin,dir,dist,angle,col,*outlineColor,duration,segmentCount),duration);
	return get_dbg_object(::DebugRenderer::DrawCone(origin,dir,dist,angle,col,duration,segmentCount),duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawSphere(
	const Vector3 &origin,float radius,const Color &col,const std::optional<Color> &outlineColor,float duration,uint32_t recursionLevel
)
{
	if(outlineColor.has_value())
		return get_dbg_object(::DebugRenderer::DrawSphere(origin,radius,col,*outlineColor,duration,recursionLevel),duration);
	return get_dbg_object(::DebugRenderer::DrawSphere(origin,radius,col,duration,recursionLevel),duration);
}
std::array<std::shared_ptr<DebugRenderer::BaseObject>,3> Lua::DebugRenderer::Client::DrawAxis(lua_State *l,const Vector3 &origin,const EulerAngles &ang,float duration)
{
	return ::DebugRenderer::DrawAxis(origin,ang,duration);
}
std::array<std::shared_ptr<DebugRenderer::BaseObject>,3> Lua::DebugRenderer::Client::DrawAxis(lua_State *l,const umath::Transform &pose,float duration)
{
	return ::DebugRenderer::DrawAxis(pose.GetOrigin(),pose.GetAngles(),duration);
}
std::array<std::shared_ptr<DebugRenderer::BaseObject>,3> Lua::DebugRenderer::Client::DrawAxis(lua_State *l,const Vector3 &origin,const Vector3 &x,const Vector3 &y,const Vector3 &z,float duration)
{
	return ::DebugRenderer::DrawAxis(origin,x,y,z,duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawText(const std::string &text,const Vector3 &origin,const Vector2 &size,const Color &col,float duration)
{
	return ::DebugRenderer::DrawText(text,origin,size,col,duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawText(const std::string &text,const Vector3 &origin,float scale,const Color &col,float duration)
{
	return ::DebugRenderer::DrawText(text,origin,scale,col,duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawText(const std::string &text,const Vector3 &origin,const Color &col,float duration)
{
	return DrawText(text,origin,1.f,col,duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPath(const std::vector<Vector3> &path,const Color &col,float duration)
{
	return ::DebugRenderer::DrawPath(path,col,duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawSpline(const std::vector<Vector3> &path,const Color &col,uint32_t numSegments,float duration,float curvature)
{
	return ::DebugRenderer::DrawSpline(path,col,numSegments,curvature,duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPlane(const umath::Plane &plane,const Color &color,float duration)
{
	return ::DebugRenderer::DrawPlane(plane,color,duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPlane(const Vector3 &n,float d,const Color &color,float duration)
{
	return ::DebugRenderer::DrawPlane(n,d,color,duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawFrustum(pragma::CCameraComponent &cam,float duration=0.f)
{
	std::vector<Vector3> points {};
	points.reserve(8u);
	cam.GetFrustumPoints(points);
	return ::DebugRenderer::DrawFrustum(points,duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawFrustum(const std::vector<Vector3> &points,float duration)
{
	return ::DebugRenderer::DrawFrustum(points,duration);
}

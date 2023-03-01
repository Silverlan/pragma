/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/libraries/s_ldebugoverlay.h"
#include "pragma/debug/debugoverlay.h"
#include "pragma/lua/classes/ldef_color.h"
#include "pragma/lua/classes/ldef_angle.h"
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/debug/debug_render_info.hpp>
#include "pragma/lua/classes/ldef_plane.h"
#include "luasystem.h"

void Lua::DebugRenderer::Server::DrawPoint(const DebugRenderInfo &renderInfo) { SDebugRenderer::DrawPoint(renderInfo.pose.GetOrigin(), renderInfo.color, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawLine(const Vector3 &start, const Vector3 &end, const DebugRenderInfo &renderInfo) { SDebugRenderer::DrawLine(renderInfo.pose.GetOrigin() + start, renderInfo.pose.GetOrigin() + end, renderInfo.color, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawBox(const Vector3 &min, const Vector3 &max, const DebugRenderInfo &renderInfo)
{
	if(renderInfo.outlineColor.has_value())
		SDebugRenderer::DrawBox(renderInfo.pose.GetOrigin(), min, max, EulerAngles {renderInfo.pose.GetRotation()}, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration);
	else
		SDebugRenderer::DrawBox(renderInfo.pose.GetOrigin(), min, max, EulerAngles {renderInfo.pose.GetRotation()}, renderInfo.color, renderInfo.duration);
}

void Lua::DebugRenderer::Server::DrawSphere(float radius, const DebugRenderInfo &renderInfo) { DrawSphere(radius, renderInfo, 1); }
void Lua::DebugRenderer::Server::DrawSphere(float radius, const DebugRenderInfo &renderInfo, uint32_t recursionLevel)
{
	if(renderInfo.outlineColor.has_value())
		SDebugRenderer::DrawSphere(renderInfo.pose.GetOrigin(), radius, renderInfo.color, renderInfo.duration, recursionLevel);
	else
		SDebugRenderer::DrawSphere(renderInfo.pose.GetOrigin(), radius, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration, recursionLevel);
}
void Lua::DebugRenderer::Server::DrawCone(float dist, const DebugRenderInfo &renderInfo)
{
	float angle = 0.f;
	uint32_t segmentCount = 12;
	if(renderInfo.outlineColor.has_value())
		SDebugRenderer::DrawCone(renderInfo.pose.GetOrigin(), uquat::forward(renderInfo.pose.GetRotation()), dist, angle, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration, segmentCount);
	else
		SDebugRenderer::DrawCone(renderInfo.pose.GetOrigin(), uquat::forward(renderInfo.pose.GetRotation()), dist, angle, renderInfo.color, renderInfo.duration, segmentCount);
}
void Lua::DebugRenderer::Server::DrawTruncatedCone(float startRadius, float dist, float endRadius, const DebugRenderInfo &renderInfo)
{
	uint32_t segmentCount = 12;
	if(renderInfo.outlineColor.has_value())
		SDebugRenderer::DrawTruncatedCone(renderInfo.pose.GetOrigin(), startRadius, uquat::forward(renderInfo.pose.GetRotation()), dist, endRadius, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration, segmentCount);
	else
		SDebugRenderer::DrawTruncatedCone(renderInfo.pose.GetOrigin(), startRadius, uquat::forward(renderInfo.pose.GetRotation()), dist, endRadius, renderInfo.color, renderInfo.duration, segmentCount);
}
void Lua::DebugRenderer::Server::DrawCylinder(float dist, float radius, const DebugRenderInfo &renderInfo)
{
	uint32_t segmentCount = 12;
	if(renderInfo.outlineColor.has_value())
		SDebugRenderer::DrawCylinder(renderInfo.pose.GetOrigin(), uquat::forward(renderInfo.pose.GetRotation()), dist, radius, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration, segmentCount);
	else
		SDebugRenderer::DrawCylinder(renderInfo.pose.GetOrigin(), uquat::forward(renderInfo.pose.GetRotation()), dist, radius, renderInfo.color, renderInfo.duration, segmentCount);
}
void Lua::DebugRenderer::Server::DrawAxis(const DebugRenderInfo &renderInfo) { SDebugRenderer::DrawAxis(renderInfo.pose.GetOrigin(), EulerAngles {renderInfo.pose.GetRotation()}, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawText(const std::string &text, const Vector2 &size, const DebugRenderInfo &renderInfo) { ::SDebugRenderer::DrawText(text, renderInfo.pose.GetOrigin(), size, renderInfo.color, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawText(const std::string &text, float size, const DebugRenderInfo &renderInfo) { ::SDebugRenderer::DrawText(text, renderInfo.pose.GetOrigin(), size, renderInfo.color, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawPath(lua_State *l, luabind::table<> points, const DebugRenderInfo &renderInfo)
{
	auto path = Lua::table_to_vector<Vector3>(l, points, 1);
	::SDebugRenderer::DrawPath(path, renderInfo.color, renderInfo.duration);
}
void Lua::DebugRenderer::Server::DrawSpline(lua_State *l, luabind::table<> points, uint32_t segmentCount, float curvature, const DebugRenderInfo &renderInfo)
{
	auto path = Lua::table_to_vector<Vector3>(l, points, 1);
	::SDebugRenderer::DrawSpline(path, renderInfo.color, segmentCount, curvature, renderInfo.duration);
}
void Lua::DebugRenderer::Server::DrawSpline(lua_State *l, luabind::table<> points, uint32_t segmentCount, const DebugRenderInfo &renderInfo) { DrawSpline(l, points, segmentCount, 1.f, renderInfo); }
void Lua::DebugRenderer::Server::DrawPlane(const DebugRenderInfo &renderInfo) { ::SDebugRenderer::DrawPlane(umath::Plane {uquat::forward(renderInfo.pose.GetRotation()), renderInfo.pose.GetOrigin()}, renderInfo.color, renderInfo.duration); }

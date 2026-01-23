// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.libraries.debug_overlay;

import :debug;

#undef DrawText

void Lua::DebugRenderer::Server::DrawPoint(const pragma::debug::DebugRenderInfo &renderInfo) { pragma::debug::SDebugRenderer::DrawPoint(renderInfo.pose.GetOrigin(), renderInfo.color, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawLine(const Vector3 &start, const Vector3 &end, const pragma::debug::DebugRenderInfo &renderInfo) { pragma::debug::SDebugRenderer::DrawLine(renderInfo.pose.GetOrigin() + start, renderInfo.pose.GetOrigin() + end, renderInfo.color, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawBox(const Vector3 &min, const Vector3 &max, const pragma::debug::DebugRenderInfo &renderInfo)
{
	if(renderInfo.outlineColor.has_value())
		pragma::debug::SDebugRenderer::DrawBox(renderInfo.pose.GetOrigin(), min, max, EulerAngles {renderInfo.pose.GetRotation()}, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration);
	else
		pragma::debug::SDebugRenderer::DrawBox(renderInfo.pose.GetOrigin(), min, max, EulerAngles {renderInfo.pose.GetRotation()}, renderInfo.color, renderInfo.duration);
}

void Lua::DebugRenderer::Server::DrawSphere(float radius, const pragma::debug::DebugRenderInfo &renderInfo) { DrawSphere(radius, renderInfo, 1); }
void Lua::DebugRenderer::Server::DrawSphere(float radius, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t recursionLevel)
{
	if(renderInfo.outlineColor.has_value())
		pragma::debug::SDebugRenderer::DrawSphere(renderInfo.pose.GetOrigin(), radius, renderInfo.color, renderInfo.duration, recursionLevel);
	else
		pragma::debug::SDebugRenderer::DrawSphere(renderInfo.pose.GetOrigin(), radius, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration, recursionLevel);
}
void Lua::DebugRenderer::Server::DrawCone(float dist, const pragma::debug::DebugRenderInfo &renderInfo)
{
	float angle = 0.f;
	uint32_t segmentCount = 12;
	if(renderInfo.outlineColor.has_value())
		pragma::debug::SDebugRenderer::DrawCone(renderInfo.pose.GetOrigin(), uquat::forward(renderInfo.pose.GetRotation()), dist, angle, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration, segmentCount);
	else
		pragma::debug::SDebugRenderer::DrawCone(renderInfo.pose.GetOrigin(), uquat::forward(renderInfo.pose.GetRotation()), dist, angle, renderInfo.color, renderInfo.duration, segmentCount);
}
void Lua::DebugRenderer::Server::DrawTruncatedCone(float startRadius, float dist, float endRadius, const pragma::debug::DebugRenderInfo &renderInfo)
{
	uint32_t segmentCount = 12;
	if(renderInfo.outlineColor.has_value())
		pragma::debug::SDebugRenderer::DrawTruncatedCone(renderInfo.pose.GetOrigin(), startRadius, uquat::forward(renderInfo.pose.GetRotation()), dist, endRadius, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration, segmentCount);
	else
		pragma::debug::SDebugRenderer::DrawTruncatedCone(renderInfo.pose.GetOrigin(), startRadius, uquat::forward(renderInfo.pose.GetRotation()), dist, endRadius, renderInfo.color, renderInfo.duration, segmentCount);
}
void Lua::DebugRenderer::Server::DrawCylinder(float dist, float radius, const pragma::debug::DebugRenderInfo &renderInfo)
{
	uint32_t segmentCount = 12;
	if(renderInfo.outlineColor.has_value())
		pragma::debug::SDebugRenderer::DrawCylinder(renderInfo.pose.GetOrigin(), uquat::forward(renderInfo.pose.GetRotation()), dist, radius, renderInfo.color, *renderInfo.outlineColor, renderInfo.duration, segmentCount);
	else
		pragma::debug::SDebugRenderer::DrawCylinder(renderInfo.pose.GetOrigin(), uquat::forward(renderInfo.pose.GetRotation()), dist, radius, renderInfo.color, renderInfo.duration, segmentCount);
}
void Lua::DebugRenderer::Server::DrawAxis(const pragma::debug::DebugRenderInfo &renderInfo) { pragma::debug::SDebugRenderer::DrawAxis(renderInfo.pose.GetOrigin(), EulerAngles {renderInfo.pose.GetRotation()}, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawText(const std::string &text, const ::Vector2 &size, const pragma::debug::DebugRenderInfo &renderInfo) { pragma::debug::SDebugRenderer::DrawText(text, renderInfo.pose.GetOrigin(), size, renderInfo.color, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawText(const std::string &text, float size, const pragma::debug::DebugRenderInfo &renderInfo) { pragma::debug::SDebugRenderer::DrawText(text, renderInfo.pose.GetOrigin(), size, renderInfo.color, renderInfo.duration); }
void Lua::DebugRenderer::Server::DrawPath(lua::State *l, luabind::table<> points, const pragma::debug::DebugRenderInfo &renderInfo)
{
	auto path = Lua::table_to_vector<Vector3>(l, points, 1);
	pragma::debug::SDebugRenderer::DrawPath(path, renderInfo.color, renderInfo.duration);
}
void Lua::DebugRenderer::Server::DrawSpline(lua::State *l, luabind::table<> points, uint32_t segmentCount, float curvature, const pragma::debug::DebugRenderInfo &renderInfo)
{
	auto path = Lua::table_to_vector<Vector3>(l, points, 1);
	pragma::debug::SDebugRenderer::DrawSpline(path, renderInfo.color, segmentCount, curvature, renderInfo.duration);
}
void Lua::DebugRenderer::Server::DrawSpline(lua::State *l, luabind::table<> points, uint32_t segmentCount, const pragma::debug::DebugRenderInfo &renderInfo) { DrawSpline(l, points, segmentCount, 1.f, renderInfo); }
void Lua::DebugRenderer::Server::DrawPlane(const pragma::debug::DebugRenderInfo &renderInfo) { pragma::debug::SDebugRenderer::DrawPlane(pragma::math::Plane {uquat::forward(renderInfo.pose.GetRotation()), renderInfo.pose.GetOrigin()}, renderInfo.color, renderInfo.duration); }

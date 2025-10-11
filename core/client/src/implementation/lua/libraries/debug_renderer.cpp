// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "mathutil/umath.h"

#include "stdafx_client.h"
#include "luasystem.h"

module pragma.client;


import :scripting.lua.libraries.debug_renderer;
import :debug;

#undef DrawText

static std::shared_ptr<DebugRenderer::BaseObject> get_dbg_object(const std::shared_ptr<DebugRenderer::BaseObject> &obj, float duration) { return (duration > 0.f) ? nullptr : obj; }
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPoints(const std::vector<::Vector3> &points, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawPoints(points, {renderInfo.color, renderInfo.duration}), renderInfo.duration); }

std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawLines(const std::vector<::Vector3> &linePoints, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawLines(linePoints, {renderInfo.color, renderInfo.duration}), renderInfo.duration); }

std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPoint(const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawPoint(renderInfo), renderInfo.duration); }

std::shared_ptr<::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawLine(const ::Vector3 &start, const ::Vector3 &end)
{
	DebugRenderInfo debugRenderInfo {};
	debugRenderInfo.duration = 12.f;
	debugRenderInfo.color = ::Color::Red;
	return DrawLine(start, end, debugRenderInfo);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawLine(const ::Vector3 &start, const ::Vector3 &end, const DebugRenderInfo &renderInfo)
{
	auto o = ::DebugRenderer::DrawLine(start, end, renderInfo);
	if(o)
		o->SetPose(renderInfo.pose);
	return get_dbg_object(o, renderInfo.duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawBox(const ::Vector3 &start, const ::Vector3 &end, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawBox(start, end, renderInfo), renderInfo.duration); }
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawMeshes(const std::vector<::Vector3> &verts, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawMesh(verts, renderInfo), renderInfo.duration); }
std::shared_ptr<::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawMesh(const ::ModelSubMesh &mesh, const DebugRenderInfo &renderInfo)
{
	if(mesh.GetGeometryType() != ::ModelSubMesh::GeometryType::Triangles)
		return nullptr;
	std::vector<::Vector3> dbgVerts;
	auto &verts = mesh.GetVertices();
	auto numIndices = mesh.GetIndexCount();
	dbgVerts.resize(numIndices);
	for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i)
		dbgVerts[i] = verts[*mesh.GetIndex(i)].position;
	return DrawMeshes(dbgVerts, renderInfo);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawTruncatedCone(float startRadius, const ::Vector3 &dir, float dist, float endRadius, const DebugRenderInfo &renderInfo, uint32_t segmentCount)
{
	return get_dbg_object(::DebugRenderer::DrawTruncatedCone(renderInfo, startRadius, dir, dist, endRadius, segmentCount), renderInfo.duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawCylinder(float radius, const ::Vector3 &dir, float dist, const DebugRenderInfo &renderInfo, uint32_t segmentCount)
{
	return get_dbg_object(::DebugRenderer::DrawCylinder(renderInfo, dir, dist, radius, segmentCount), renderInfo.duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawCone(const ::Vector3 &dir, float dist, float angle, const DebugRenderInfo &renderInfo, uint32_t segmentCount)
{
	return get_dbg_object(::DebugRenderer::DrawCone(renderInfo, dir, dist, angle, segmentCount), renderInfo.duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawSphere(float radius, const DebugRenderInfo &renderInfo, uint32_t recursionLevel) { return get_dbg_object(::DebugRenderer::DrawSphere(renderInfo, radius, recursionLevel), renderInfo.duration); }
std::array<std::shared_ptr<DebugRenderer::BaseObject>, 3> Lua::DebugRenderer::Client::DrawAxis(const DebugRenderInfo &renderInfo)
{
	auto objs = ::DebugRenderer::DrawAxis(renderInfo);
	if(renderInfo.duration > 0.f)
		return {};
	return objs;
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawText(const std::string &text, const ::Vector2 &size, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawText(renderInfo, text, size), renderInfo.duration); }
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawText(const std::string &text, float scale, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawText(renderInfo, text, scale), renderInfo.duration); }
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawText(const std::string &text, const DebugRenderInfo &renderInfo) { return DrawText(text, 1.f, renderInfo); }
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPath(const std::vector<::Vector3> &path, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawPath(path, {renderInfo.color, renderInfo.duration}), renderInfo.duration); }
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawSpline(const std::vector<::Vector3> &path, uint32_t numSegments, const DebugRenderInfo &renderInfo, float curvature)
{
	return get_dbg_object(::DebugRenderer::DrawSpline(path, numSegments, curvature, {renderInfo.color, renderInfo.duration}), renderInfo.duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPlane(const umath::Plane &plane, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawPlane(plane, {renderInfo.color, renderInfo.duration}), renderInfo.duration); }
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPlane(const ::Vector3 &n, float d, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawPlane(n, d, {renderInfo.color, renderInfo.duration}), renderInfo.duration); }
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawFrustum(pragma::CCameraComponent &cam, const DebugRenderInfo &renderInfo)
{
	std::vector<::Vector3> points {};
	points.reserve(8u);
	cam.GetFrustumPoints(points);
	DebugRenderInfo sub {};
	sub.SetDuration(renderInfo.duration);
	return get_dbg_object(::DebugRenderer::DrawFrustum(points, sub), renderInfo.duration);
}
std::shared_ptr<DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawFrustum(const std::vector<::Vector3> &points, const DebugRenderInfo &renderInfo) { return get_dbg_object(::DebugRenderer::DrawFrustum(points, renderInfo), renderInfo.duration); }

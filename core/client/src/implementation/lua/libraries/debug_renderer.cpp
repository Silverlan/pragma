// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.debug_renderer;
import :debug;

#undef DrawText

static std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> get_dbg_object(const std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> &obj, float duration) { return (duration > 0.f) ? nullptr : obj; }
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPoints(const std::vector<Vector3> &points, const pragma::debug::DebugRenderInfo &renderInfo)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawPoints(points, {renderInfo.color, renderInfo.duration}), renderInfo.duration);
}

std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawLines(const std::vector<Vector3> &linePoints, const pragma::debug::DebugRenderInfo &renderInfo)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawLines(linePoints, {renderInfo.color, renderInfo.duration}), renderInfo.duration);
}

std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPoint(const pragma::debug::DebugRenderInfo &renderInfo) { return get_dbg_object(pragma::debug::DebugRenderer::DrawPoint(renderInfo), renderInfo.duration); }

std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawLine(const Vector3 &start, const Vector3 &end)
{
	pragma::debug::DebugRenderInfo debugRenderInfo {};
	debugRenderInfo.duration = 12.f;
	debugRenderInfo.color = colors::Red;
	return DrawLine(start, end, debugRenderInfo);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawLine(const Vector3 &start, const Vector3 &end, const pragma::debug::DebugRenderInfo &renderInfo)
{
	auto o = pragma::debug::DebugRenderer::DrawLine(start, end, renderInfo);
	if(o)
		o->SetPose(renderInfo.pose);
	return get_dbg_object(o, renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawBox(const Vector3 &start, const Vector3 &end, const pragma::debug::DebugRenderInfo &renderInfo)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawBox(start, end, renderInfo), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawMeshes(const std::vector<Vector3> &verts, const pragma::debug::DebugRenderInfo &renderInfo) { return get_dbg_object(pragma::debug::DebugRenderer::DrawMesh(verts, renderInfo), renderInfo.duration); }
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawMesh(const pragma::geometry::ModelSubMesh &mesh, const pragma::debug::DebugRenderInfo &renderInfo)
{
	if(mesh.GetGeometryType() != pragma::geometry::ModelSubMesh::GeometryType::Triangles)
		return nullptr;
	std::vector<Vector3> dbgVerts;
	auto &verts = mesh.GetVertices();
	auto numIndices = mesh.GetIndexCount();
	dbgVerts.resize(numIndices);
	for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i)
		dbgVerts[i] = verts[*mesh.GetIndex(i)].position;
	return DrawMeshes(dbgVerts, renderInfo);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawTruncatedCone(float startRadius, const Vector3 &dir, float dist, float endRadius, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t segmentCount)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawTruncatedCone(renderInfo, startRadius, dir, dist, endRadius, segmentCount), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawCylinder(float radius, const Vector3 &dir, float dist, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t segmentCount)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawCylinder(renderInfo, dir, dist, radius, segmentCount), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawCone(const Vector3 &dir, float dist, float angle, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t segmentCount)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawCone(renderInfo, dir, dist, angle, segmentCount), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawSphere(float radius, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t recursionLevel)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawSphere(renderInfo, radius, recursionLevel), renderInfo.duration);
}
std::array<std::shared_ptr<pragma::debug::DebugRenderer::BaseObject>, 3> Lua::DebugRenderer::Client::DrawAxis(const pragma::debug::DebugRenderInfo &renderInfo)
{
	auto objs = pragma::debug::DebugRenderer::DrawAxis(renderInfo);
	if(renderInfo.duration > 0.f)
		return {};
	return objs;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawText(const std::string &text, const ::Vector2 &size, const pragma::debug::DebugRenderInfo &renderInfo)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawText(renderInfo, text, size), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawText(const std::string &text, float scale, const pragma::debug::DebugRenderInfo &renderInfo)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawText(renderInfo, text, scale), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawText(const std::string &text, const pragma::debug::DebugRenderInfo &renderInfo) { return DrawText(text, 1.f, renderInfo); }
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPath(const std::vector<Vector3> &path, const pragma::debug::DebugRenderInfo &renderInfo)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawPath(path, {renderInfo.color, renderInfo.duration}), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawSpline(const std::vector<Vector3> &path, uint32_t numSegments, const pragma::debug::DebugRenderInfo &renderInfo, float curvature)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawSpline(path, numSegments, curvature, {renderInfo.color, renderInfo.duration}), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPlane(const pragma::math::Plane &plane, const pragma::debug::DebugRenderInfo &renderInfo)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawPlane(plane, {renderInfo.color, renderInfo.duration}), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawPlane(const Vector3 &n, float d, const pragma::debug::DebugRenderInfo &renderInfo)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawPlane(n, d, {renderInfo.color, renderInfo.duration}), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawFrustum(pragma::CCameraComponent &cam, const pragma::debug::DebugRenderInfo &renderInfo)
{
	std::vector<Vector3> points {};
	points.reserve(8u);
	cam.GetFrustumPoints(points);
	pragma::debug::DebugRenderInfo sub {};
	sub.SetDuration(renderInfo.duration);
	return get_dbg_object(pragma::debug::DebugRenderer::DrawFrustum(points, sub), renderInfo.duration);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> Lua::DebugRenderer::Client::DrawFrustum(const std::vector<Vector3> &points, const pragma::debug::DebugRenderInfo &renderInfo)
{
	return get_dbg_object(pragma::debug::DebugRenderer::DrawFrustum(points, renderInfo), renderInfo.duration);
}

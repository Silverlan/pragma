// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.debug_renderer;
export import :debug;

#undef DrawText

export namespace Lua {
	namespace DebugRenderer {
		namespace Client {
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawPoints(const std::vector<Vector3> &points, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawLines(const std::vector<Vector3> &linePoints, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawPoint(const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawLine(const Vector3 &start, const Vector3 &end, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawLine(const Vector3 &start, const Vector3 &end);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawBox(const Vector3 &start, const Vector3 &end, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawMeshes(const std::vector<Vector3> &verts, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawMesh(const pragma::geometry::ModelSubMesh &mesh, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawSphere(float radius, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t recursionLevel = 1);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawTruncatedCone(float startRadius, const Vector3 &dir, float dist, float endRadius, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t segmentCount = 12);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawCylinder(float radius, const Vector3 &dir, float dist, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t segmentCount = 12);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawCone(const Vector3 &dir, float dist, float angle, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t segmentCount = 12);
			DLLCLIENT std::array<std::shared_ptr<pragma::debug::DebugRenderer::BaseObject>, 3> DrawAxis(const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawText(const std::string &text, const ::Vector2 &size, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawText(const std::string &text, float scale, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawText(const std::string &text, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawPath(const std::vector<Vector3> &path, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawSpline(const std::vector<Vector3> &path, uint32_t numSegments, const pragma::debug::DebugRenderInfo &renderInfo, float curvature = 1.f);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawPlane(const pragma::math::Plane &plane, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawPlane(const Vector3 &n, float d, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawFrustum(pragma::CCameraComponent &cam, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> DrawFrustum(const std::vector<Vector3> &points, const pragma::debug::DebugRenderInfo &renderInfo);
		};
	};
};

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "mathutil/umath.h"



export module pragma.client:scripting.lua.libraries.debug_renderer;
import :debug;

export namespace Lua {
	namespace DebugRenderer {
		namespace Client {
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPoints(const std::vector<Vector3> &points, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawLines(const std::vector<Vector3> &linePoints, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPoint(const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawLine(const Vector3 &start, const Vector3 &end, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawLine(const Vector3 &start, const Vector3 &end);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawBox(const Vector3 &start, const Vector3 &end, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawMeshes(const std::vector<Vector3> &verts, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawMesh(const ::ModelSubMesh &mesh, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawSphere(float radius, const DebugRenderInfo &renderInfo, uint32_t recursionLevel = 1);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawTruncatedCone(float startRadius, const Vector3 &dir, float dist, float endRadius, const DebugRenderInfo &renderInfo, uint32_t segmentCount = 12);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawCylinder(float radius, const Vector3 &dir, float dist, const DebugRenderInfo &renderInfo, uint32_t segmentCount = 12);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawCone(const Vector3 &dir, float dist, float angle, const DebugRenderInfo &renderInfo, uint32_t segmentCount = 12);
			DLLCLIENT std::array<std::shared_ptr<::DebugRenderer::BaseObject>, 3> DrawAxis(const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawText(const std::string &text, const ::Vector2 &size, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawText(const std::string &text, float scale, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawText(const std::string &text, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPath(const std::vector<Vector3> &path, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawSpline(const std::vector<Vector3> &path, uint32_t numSegments, const DebugRenderInfo &renderInfo, float curvature = 1.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPlane(const umath::Plane &plane, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPlane(const Vector3 &n, float d, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawFrustum(pragma::CCameraComponent &cam, const DebugRenderInfo &renderInfo);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawFrustum(const std::vector<Vector3> &points, const DebugRenderInfo &renderInfo);
		};
	};
};

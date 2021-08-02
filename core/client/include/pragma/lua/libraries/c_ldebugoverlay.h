/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LDEBUGOVERLAY_H__
#define __C_LDEBUGOVERLAY_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/debug/c_debugoverlay.h"

namespace Lua
{
	namespace DebugRenderer
	{
		namespace Client
		{
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPoints(const std::vector<Vector3> &points,const Color &col,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawLines(const std::vector<Vector3> &linePoints,const Color &col,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPoint(const Vector3 &pos,const Color &color,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawLine(const Vector3 &start,const Vector3 &end,const Color &color,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawBox(
				const Vector3 &center,const Vector3 &start,const Vector3 &end,const Color &color,const std::optional<Color> &colOutline,float duration=0.f,const EulerAngles &angles={}
			);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawBox(
				const Vector3 &center,const Vector3 &start,const Color &color,const std::optional<Color> &colOutline,float duration=0.f,const EulerAngles &angles={}
			);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawMeshes(const std::vector<Vector3> &verts,const Color &color,const std::optional<Color> &outlineColor,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawSphere(
				const Vector3 &origin,float radius,const Color &col,const std::optional<Color> &outlineColor,float duration=0.f,uint32_t recursionLevel=1
			);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawTruncatedCone(
				const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,const Color &col,const std::optional<Color> &outlineColor,float duration=0.f,uint32_t segmentCount=12
			);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawCylinder(
				const Vector3 &origin,float radius,const Vector3 &dir,float dist,const Color &color,const std::optional<Color> &outlineColor,float duration=0.f,uint32_t segmentCount=12
			);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawCone(
				const Vector3 &origin,const Vector3 &dir,float dist,float angle,const Color &col,const std::optional<Color> &outlineColor,float duration=0.f,uint32_t segmentCount=12
			);
			DLLCLIENT std::array<std::shared_ptr<::DebugRenderer::BaseObject>,3> DrawAxis(const Vector3 &origin,const EulerAngles &ang,float duration=0.f);
			DLLCLIENT std::array<std::shared_ptr<::DebugRenderer::BaseObject>,3> DrawAxis(const umath::Transform &pose,float duration=0.f);
			DLLCLIENT std::array<std::shared_ptr<::DebugRenderer::BaseObject>,3> DrawAxis(const Vector3 &origin,const Vector3 &x,const Vector3 &y,const Vector3 &z,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawText(const std::string &text,const Vector3 &origin,const Vector2 &size,const Color &col,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawText(const std::string &text,const Vector3 &origin,float scale,const Color &col,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawText(const std::string &text,const Vector3 &origin,const Color &col,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPath(const std::vector<Vector3> &path,const Color &col,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawSpline(const std::vector<Vector3> &path,const Color &col,uint32_t numSegments,float duration=0.f,float curvature=1.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPlane(const umath::Plane &plane,const Color &color,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawPlane(const Vector3 &n,float d,const Color &color,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawFrustum(pragma::CCameraComponent &cam,float duration=0.f);
			DLLCLIENT std::shared_ptr<::DebugRenderer::BaseObject> DrawFrustum(const std::vector<Vector3> &points,float duration=0.f);
		};
	};
};

#endif
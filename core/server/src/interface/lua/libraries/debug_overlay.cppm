// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.server:scripting.lua.libraries.debug_overlay;

export import pragma.shared;
export import luabind;

#undef DrawText

export namespace Lua {
	namespace DebugRenderer {
		namespace Server {
			DLLSERVER void DrawPoint(const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawLine(const Vector3 &start, const Vector3 &end, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawBox(const Vector3 &min, const Vector3 &max, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawSphere(float radius, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawSphere(float radius, const DebugRenderInfo &renderInfo, uint32_t recursionLevel);
			DLLSERVER void DrawCone(float dist, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawTruncatedCone(float startRadius, float dist, float endRadius, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawCylinder(float dist, float radius, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawAxis(const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawText(const std::string &text, const ::Vector2 &size, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawText(const std::string &text, float size, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawPath(lua::State *l, luabind::table<> points, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawSpline(lua::State *l, luabind::table<> points, uint32_t segmentCount, float curvature, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawSpline(lua::State *l, luabind::table<> points, uint32_t segmentCount, const DebugRenderInfo &renderInfo);
			DLLSERVER void DrawPlane(const DebugRenderInfo &renderInfo);
		};
	};
};

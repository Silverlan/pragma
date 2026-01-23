// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.libraries.debug_overlay;

export import pragma.shared;
export import luabind;

#undef DrawText

export namespace Lua {
	namespace DebugRenderer {
		namespace Server {
			DLLSERVER void DrawPoint(const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawLine(const Vector3 &start, const Vector3 &end, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawBox(const Vector3 &min, const Vector3 &max, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawSphere(float radius, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawSphere(float radius, const pragma::debug::DebugRenderInfo &renderInfo, uint32_t recursionLevel);
			DLLSERVER void DrawCone(float dist, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawTruncatedCone(float startRadius, float dist, float endRadius, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawCylinder(float dist, float radius, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawAxis(const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawText(const std::string &text, const ::Vector2 &size, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawText(const std::string &text, float size, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawPath(lua::State *l, luabind::table<> points, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawSpline(lua::State *l, luabind::table<> points, uint32_t segmentCount, float curvature, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawSpline(lua::State *l, luabind::table<> points, uint32_t segmentCount, const pragma::debug::DebugRenderInfo &renderInfo);
			DLLSERVER void DrawPlane(const pragma::debug::DebugRenderInfo &renderInfo);
		};
	};
};

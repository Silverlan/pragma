// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:debug.overlay;

export import pragma.math;

#undef DrawText

export namespace pragma::debug {
	namespace SDebugRenderer {
		void DrawPoint(const Vector3 &pos, const Color &color, float duration = 0.f);
		void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration = 0.f);
		void DrawBox(const Vector3 &center, const Vector3 &min, const Vector3 &max, const EulerAngles &ang, const Color &color, const Color &colorOutline, float duration = 0.f);
		void DrawBox(const Vector3 &start, const Vector3 &end, const Color &color, float duration = 0.f);
		void DrawBox(const Vector3 &center, const Vector3 &min, const Vector3 &max, const Color &color, float duration = 0.f);
		void DrawBox(const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &color, const Color &colorOutline, float duration = 0.f);
		void DrawBox(const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &color, float duration = 0.f);
		void DrawBox(const Vector3 &center, const Vector3 &min, const Vector3 &max, const EulerAngles &ang, const Color &color, float duration = 0.f);
		void DrawText(const std::string &text, const Vector3 &pos, const Vector2 &worldSize, const Color &color, float duration = 0.f);
		void DrawText(const std::string &text, const Vector3 &pos, float sizeScale, const Color &color, float duration = 0.f);
		void DrawText(const std::string &text, const Vector3 &pos, const Vector2 &worldSize, float duration = 0.f);
		void DrawText(const std::string &text, const Vector3 &pos, float sizeScale, float duration = 0.f);
		void DrawSphere(const Vector3 &origin, float radius, const Color &color, const Color &outlineColor, float duration = 0.f, uint32_t recursionLevel = 1);
		void DrawSphere(const Vector3 &origin, float radius, const Color &color, float duration = 0.f, uint32_t recursionLevel = 1);
		void DrawTruncatedCone(const Vector3 &origin, float startRadius, const Vector3 &dir, float dist, float endRadius, const Color &color, const Color &outlineColor, float duration = 0.f, uint32_t segmentCount = 12);
		void DrawTruncatedCone(const Vector3 &origin, float startRadius, const Vector3 &dir, float dist, float endRadius, const Color &color, float duration = 0.f, uint32_t segmentCount = 12);
		void DrawCylinder(const Vector3 &origin, const Vector3 &dir, float dist, float radius, const Color &color, float duration = 0.f, uint32_t segmentCount = 12);
		void DrawCylinder(const Vector3 &origin, const Vector3 &dir, float dist, float radius, const Color &color, const Color &outlineColor, float duration = 0.f, uint32_t segmentCount = 12);
		void DrawCone(const Vector3 &origin, const Vector3 &dir, float dist, float angle, const Color &color, const Color &outlineColor, float duration = 0.f, uint32_t segmentCount = 12);
		void DrawCone(const Vector3 &origin, const Vector3 &dir, float dist, float angle, const Color &color, float duration = 0.f, uint32_t segmentCount = 12);
		void DrawAxis(const Vector3 &origin, const EulerAngles &ang, float duration = 0.f);
		void DrawAxis(const Vector3 &origin, float duration = 0.f);
		void DrawPath(const std::vector<Vector3> &path, const Color &color, float duration = 0.f);
		void DrawSpline(const std::vector<Vector3> &path, const Color &color, uint32_t segmentCount, float curvature = 1.f, float duration = 0.f);
		void DrawPlane(const Vector3 &n, float dist, const Color &color, float duration = 0.f);
		void DrawPlane(const math::Plane &plane, const Color &color, float duration = 0.f);
		void DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration = 0.f);
	};
}

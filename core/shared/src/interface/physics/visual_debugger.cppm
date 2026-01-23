// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

#undef DrawText

export module pragma.shared:physics.visual_debugger;

export import pragma.math;

export namespace pragma::physics {
	class DLLNETWORK IVisualDebugger {
	  public:
		enum class DebugMode : uint32_t { None = 0, Wireframe, Constraints, Normals, All };
		/* // Bullet: 
		auto mode = btIDebugDraw::DBG_DrawAabb |
			btIDebugDraw::DBG_DrawConstraintLimits |
			btIDebugDraw::DBG_DrawConstraints |
			btIDebugDraw::DBG_DrawContactPoints |
			btIDebugDraw::DBG_DrawFeaturesText |
			btIDebugDraw::DBG_DrawFrames |
			btIDebugDraw::DBG_DrawNormals |
			btIDebugDraw::DBG_DrawText |
			btIDebugDraw::DBG_DrawWireframe |
			btIDebugDraw::DBG_EnableCCD;
		if(val == 2)
			mode = btIDebugDraw::DBG_DrawWireframe;
		else if(val == 3)
			mode = btIDebugDraw::DBG_DrawConstraints;
		else if(val == 4)
			mode = btIDebugDraw::DBG_DrawNormals;
			*/
		IVisualDebugger() = default;
		virtual ~IVisualDebugger() = default;
		IVisualDebugger(IVisualDebugger &) = delete;
		IVisualDebugger &operator=(const IVisualDebugger &) = delete;

		void DrawLine(const Vector3 &from, const Vector3 &to, const Color &color);
		virtual void DrawLine(const Vector3 &from, const Vector3 &to, const Color &fromColor, const Color &toColor) = 0;
		virtual void DrawPoint(const Vector3 &pos, const Color &color) = 0;
		virtual void DrawTriangle(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Color &c0, const Color &c1, const Color &c2) = 0;
		virtual void ReportErrorWarning(const std::string &str) = 0;
		virtual void DrawText(const std::string &str, const Vector3 &location, const Color &color, float size) = 0;
		virtual void Reset() = 0;
		virtual void Flush() = 0;

		void SetDebugMode(DebugMode debugMode);
	  private:
		DebugMode m_debugMode = DebugMode::None;
	};
};

// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:rendering.shader_graph.node_camera;

import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT CameraNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *OUT_POSITION = "position";
		static constexpr const char *OUT_FOV = "fov";
		static constexpr const char *OUT_NEARZ = "nearZ";
		static constexpr const char *OUT_FARZ = "farZ";
		static constexpr const char *OUT_VIEW_MATRIX = "viewMatrix";
		static constexpr const char *OUT_PROJECTION_MATRIX = "projectionMatrix";
		static constexpr const char *OUT_VIEW_PROJECTION_MATRIX = "viewProjectionMatrix";

		CameraNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#if 0
layout(std140, LAYOUT_ID(SCENE, RENDER_SETTINGS)) uniform RenderSettings
{
	vec4 posCam; // w component is fov
	int flags;
	float shadowRatioX;
	float shadowRatioY;
	float nearZ;
	float farZ;
	int shaderQuality; // 1 = lowest, 10 = highest
}
u_renderSettings;
#endif

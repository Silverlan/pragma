// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_camera;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT CameraNode : public shadergraph::Node {
	  public:
		static constexpr const char *OUT_POSITION = "position";
		static constexpr const char *OUT_FOV = "fov";
		static constexpr const char *OUT_NEARZ = "nearZ";
		static constexpr const char *OUT_FARZ = "farZ";
		static constexpr const char *OUT_VIEW_MATRIX = "viewMatrix";
		static constexpr const char *OUT_PROJECTION_MATRIX = "projectionMatrix";
		static constexpr const char *OUT_VIEW_PROJECTION_MATRIX = "viewProjectionMatrix";

		CameraNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
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

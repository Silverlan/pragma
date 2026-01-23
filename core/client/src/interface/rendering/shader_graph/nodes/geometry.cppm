// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_geometry;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT GeometryNode : public shadergraph::Node {
	  public:
		static constexpr const char *OUT_POSITION_WS = "position_ws";
		static constexpr const char *OUT_NORMAL_WS = "normal_ws";
		static constexpr const char *OUT_NORMAL_CS = "normal_cs";
		static constexpr const char *OUT_TANGENT_WS = "tangent_ws";

		GeometryNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};

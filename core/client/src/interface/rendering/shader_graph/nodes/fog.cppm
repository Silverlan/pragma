// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_fog;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT FogNode : public shadergraph::Node {
	  public:
		static constexpr const char *OUT_COLOR = "color";
		static constexpr const char *OUT_START_DISTANCE = "startDistance";
		static constexpr const char *OUT_END_DISTANCE = "endDistance";
		static constexpr const char *OUT_DENSITY = "density";

		FogNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};

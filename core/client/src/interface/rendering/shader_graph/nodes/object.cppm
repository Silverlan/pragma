// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_object;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT ObjectNode : public shadergraph::Node {
	  public:
		static constexpr const char *OUT_MODEL_MATRIX = "modelMatrix";
		static constexpr const char *OUT_COLOR = "color";

		ObjectNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};

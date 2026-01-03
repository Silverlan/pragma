// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_time;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT TimeNode : public shadergraph::Node {
	  public:
		static constexpr const char *OUT_TIME = "time";
		static constexpr const char *OUT_DELTA_TIME = "deltaTime";
		static constexpr const char *OUT_REAL_TIME = "realTime";
		static constexpr const char *OUT_DELTA_REAL_TIME = "deltaRealTime";

		TimeNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};

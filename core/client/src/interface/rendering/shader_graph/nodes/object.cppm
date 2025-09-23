// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.shader_graph.node_object;

import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT ObjectNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *OUT_MODEL_MATRIX = "modelMatrix";
		static constexpr const char *OUT_COLOR = "color";

		ObjectNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

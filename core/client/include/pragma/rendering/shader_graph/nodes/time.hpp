// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_SHADER_GRAPH_NODES_TIME_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_TIME_HPP__

#include "pragma/clientdefinitions.h"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT TimeNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *OUT_TIME = "time";
		static constexpr const char *OUT_DELTA_TIME = "deltaTime";
		static constexpr const char *OUT_REAL_TIME = "realTime";
		static constexpr const char *OUT_DELTA_REAL_TIME = "deltaRealTime";

		TimeNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

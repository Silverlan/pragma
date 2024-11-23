/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

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

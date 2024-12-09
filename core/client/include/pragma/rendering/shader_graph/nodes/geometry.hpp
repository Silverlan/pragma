/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_SHADER_GRAPH_NODES_GEOMETRY_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_GEOMETRY_HPP__

#include "pragma/clientdefinitions.h"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT GeometryNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *OUT_POSITION_WS = "position_ws";
		static constexpr const char *OUT_NORMAL_WS = "normal_ws";
		static constexpr const char *OUT_NORMAL_CS = "normal_cs";
		static constexpr const char *OUT_TANGENT_WS = "tangent_ws";

		GeometryNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_SHADER_GRAPH_NODES_INPUT_PARAMETER_VEC3_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_INPUT_PARAMETER_VEC3_HPP__

#include "pragma/rendering/shader_graph/nodes/base_input_parameter.hpp"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT InputParameterVec3Node : public BaseInputParameterNode {
	  public:
		InputParameterVec3Node(const std::string_view &type);
		virtual pragma::shadergraph::DataType GetParameterType() const override { return pragma::shadergraph::DataType::Vector; }
		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

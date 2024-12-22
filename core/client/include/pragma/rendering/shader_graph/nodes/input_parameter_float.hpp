/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_SHADER_GRAPH_NODES_INPUT_PARAMETER_FLOAT_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_INPUT_PARAMETER_FLOAT_HPP__

#include "pragma/clientdefinitions.h"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT InputParameterFloatNode : public pragma::shadergraph::Node {
	  public:
		enum class Scope : uint32_t {
			Global = 0,
			Object,
			Material,
		};

		static constexpr const char *CONST_NAME = "name";
		static constexpr const char *CONST_SCOPE = "scope";
		static constexpr const char *CONST_DEFAULT = "default";
		static constexpr const char *CONST_MIN = "min";
		static constexpr const char *CONST_MAX = "max";
		static constexpr const char *CONST_STEP_SIZE = "stepSize";

		static constexpr const char *OUT_VALUE = "value";

		InputParameterFloatNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

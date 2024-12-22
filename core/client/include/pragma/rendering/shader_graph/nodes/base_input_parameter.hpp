/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_BASE_SHADER_GRAPH_NODES_INPUT_PARAMETER_HPP__
#define __PRAGMA_BASE_SHADER_GRAPH_NODES_INPUT_PARAMETER_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <string_view>
#include <string>

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT BaseInputParameterNode : public pragma::shadergraph::Node {
	  public:
		enum class Scope : uint32_t {
			Global = 0,
			Object,
			Material,
		};

		static constexpr const char *CONST_NAME = "name";
		static constexpr const char *CONST_SCOPE = "scope";
		static constexpr const char *CONST_DEFAULT = "default";

		static constexpr const char *OUT_VALUE = "value";

		BaseInputParameterNode(const std::string_view &type);
		virtual pragma::shadergraph::DataType GetParameterType() const = 0;
		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

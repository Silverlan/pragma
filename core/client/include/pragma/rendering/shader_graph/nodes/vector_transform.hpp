// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_SHADER_GRAPH_NODES_VECTOR_TRANSFORM_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_VECTOR_TRANSFORM_HPP__

#include "pragma/clientdefinitions.h"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT VectorTransformNode : public pragma::shadergraph::Node {
	  public:
		enum class Type : uint8_t {
			Vector = 0,
			Point,
			Normal,
		};

		enum class Space : uint8_t {
			World = 0,
			Object,
			Camera,
		};

		static constexpr const char *IN_TRANSFORM_TYPE = "transformType";
		static constexpr const char *IN_CONVERT_FROM = "convertForm";
		static constexpr const char *IN_CONVERT_TO = "convertTo";
		static constexpr const char *IN_VECTOR = "vector";

		static constexpr const char *OUT_VECTOR = "vector";

		VectorTransformNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

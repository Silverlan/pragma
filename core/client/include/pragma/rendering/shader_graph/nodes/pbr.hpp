// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_SHADER_GRAPH_NODES_PBR_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_PBR_HPP__

#include "pragma/clientdefinitions.h"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT PbrNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *IN_ALBEDO_COLOR = "albedoColor";
		static constexpr const char *IN_METALNESS = "metalness";
		static constexpr const char *IN_ROUGHNESS = "roughness";
		static constexpr const char *IN_AMBIENT_OCCLUSION = "ambientOcclusion";

		static constexpr const char *OUT_COLOR = "color";

		PbrNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

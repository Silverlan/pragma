/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_SHADER_GRAPH_NODES_LIGHTMAP_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_LIGHTMAP_HPP__

#include "pragma/clientdefinitions.h"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT LightmapNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *OUT_LIGHT_MAP = "lightMap";
		static constexpr const char *OUT_LIGHT_MAP_INDIRECT = "lightMapIndirect";
		static constexpr const char *OUT_LIGHT_MAP_DOMINANT = "lightMapDominant";

		LightmapNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

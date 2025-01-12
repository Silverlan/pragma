/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_SHADER_GRAPH_NODES_SCENE_OUTPUT_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_SCENE_OUTPUT_HPP__

#include "pragma/clientdefinitions.h"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT SceneOutputNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *IN_COLOR = "color";
		static constexpr const char *IN_ALPHA = "alpha";
		static constexpr const char *IN_ALPHA_CUTOFF = "alphaCutoff";
		static constexpr const char *IN_BLOOM_FACTOR = "bloomFactor";
		static constexpr const char *IN_EMISSIVE_COLOR = "emissiveColor";
		static constexpr const char *CONST_ALPHA_MODE = "alphaMode";
		// TODO: Only allow one of these!
		SceneOutputNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

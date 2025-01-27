/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_SHADER_GRAPH_NODES_TOON_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_TOON_HPP__

#include "pragma/clientdefinitions.h"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT ToonNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *IN_ALBEDO_COLOR = "albedoColor";
		static constexpr const char *IN_ALPHA = "alpha";
		static constexpr const char *IN_SPECULAR_COLOR = "specularColor";
		static constexpr const char *IN_GLOSSINESS = "glossiness";
		static constexpr const char *IN_RIM_AMOUNT = "rimAmount";
		static constexpr const char *IN_RIM_COLOR = "rimColor";
		static constexpr const char *IN_MATCAP_TEXTURE = "matcapTexture";
		static constexpr const char *IN_RAMP_TEXTURE = "rampTexture";

		static constexpr const char *OUT_COLOR = "color";

		ToonNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#endif

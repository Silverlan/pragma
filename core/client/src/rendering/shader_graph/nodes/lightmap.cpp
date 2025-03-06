/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/lightmap.hpp"

using namespace pragma::rendering::shader_graph;

LightmapNode::LightmapNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_TEXTURE} { AddModuleDependency("lightmap"); }

std::string LightmapNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	return code.str();
}

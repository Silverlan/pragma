// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/lightmap.hpp"

using namespace pragma::rendering::shader_graph;

LightmapNode::LightmapNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_TEXTURE} { AddModuleDependency("lightmap"); }

std::string LightmapNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	return code.str();
}

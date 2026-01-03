// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_lightmap;

using namespace pragma::rendering::shader_graph;

LightmapNode::LightmapNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_TEXTURE} { AddModuleDependency("lightmap"); }

std::string LightmapNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	return code.str();
}

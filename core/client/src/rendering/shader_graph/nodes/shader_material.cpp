/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/shader_material.hpp"

using namespace pragma::rendering::shader_graph;

ShaderMaterialNode::ShaderMaterialNode(const std::string_view &type, const pragma::rendering::shader_material::ShaderMaterial &shaderMaterial) : pragma::shadergraph::Node {type}, m_shaderMaterial {shaderMaterial}
{
	for(auto &prop : m_shaderMaterial.properties) {
		auto socketType = pragma::shadergraph::to_socket_type(prop.type);
		if(socketType == pragma::shadergraph::SocketType::Invalid)
			continue;
		AddOutput(prop.name.c_str(), socketType);
	}
}
std::string ShaderMaterialNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	// TODO: Only write output var is output is set?
	// get_mat_x();
	std::ostringstream code;
	for(auto &prop : m_shaderMaterial.properties) {
		auto socketType = pragma::shadergraph::to_socket_type(prop.type);
		if(socketType == pragma::shadergraph::SocketType::Invalid)
			continue;
		if(!instance.IsOutputLinked(prop.name))
			continue;
		auto *glslType = pragma::shadergraph::to_glsl_type(socketType);
		code << glslType << " ";
		code << instance.GetOutputVarName(prop.name.c_str()) << " = ";
		code << "get_mat_" << prop.name.c_str() << "();\n";
	}
	return code.str();
}

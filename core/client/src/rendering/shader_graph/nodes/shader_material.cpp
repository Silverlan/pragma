/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/shader_material.hpp"

using namespace pragma::rendering::shader_graph;

ShaderMaterialNode::ShaderMaterialNode(const std::string_view &type, const pragma::rendering::shader_material::ShaderMaterial &shaderMaterial) : pragma::shadergraph::Node {type, pragma::shadergraph::CATEGORY_TEXTURE}, m_shaderMaterial {shaderMaterial}
{
	for(auto &tex : m_shaderMaterial.textures) {
		auto name = ustring::to_camel_case(tex.name);
		AddOutput(name, pragma::shadergraph::DataType::String);
	}

	for(auto &prop : m_shaderMaterial.properties) {
		if(umath::is_flag_set(prop.propertyFlags, pragma::rendering::Property::Flags::HideInEditor))
			continue;
		auto socketType = pragma::shadergraph::to_data_type(pragma::shadergraph::to_udm_type(prop.parameter.type));
		if(socketType == pragma::shadergraph::DataType::Invalid)
			continue;
		AddOutput(ustring::to_camel_case(prop.parameter.name), socketType);
	}
}
std::string ShaderMaterialNode::GetTextureVariableName(const pragma::shadergraph::OutputSocket &socket) const { return pragma::rendering::shader_material::ShaderMaterial::GetTextureUniformVariableName(socket.GetSocket().name); }
std::string ShaderMaterialNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	// TODO: Only write output var is output is set?
	// get_mat_x();
	std::ostringstream code;
	for(auto &prop : m_shaderMaterial.properties) {
		auto socketType = prop.parameter.type;
		if(socketType == pragma::shadergraph::DataType::Invalid)
			continue;
		auto socketName = ustring::to_camel_case(prop.parameter.name);
		if(!gn.IsOutputLinked(socketName))
			continue;
		auto *glslType = pragma::shadergraph::to_glsl_type(socketType);
		code << glslType << " ";
		code << gn.GetOutputVarName(socketName) << " = ";
		code << "get_mat_" << prop.parameter.name.c_str() << "();\n";
	}
	return code.str();
}

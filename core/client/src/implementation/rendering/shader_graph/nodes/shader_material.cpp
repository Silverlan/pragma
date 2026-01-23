// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_shader_material;

using namespace pragma::rendering::shader_graph;

ShaderMaterialNode::ShaderMaterialNode(const std::string_view &type, const shader_material::ShaderMaterial &shaderMaterial) : Node {type, shadergraph::CATEGORY_TEXTURE}, m_shaderMaterial {shaderMaterial}
{
	for(auto &tex : m_shaderMaterial.textures) {
		auto name = string::to_camel_case(tex.name);
		AddOutput(name, shadergraph::DataType::String);
	}

	for(auto &prop : m_shaderMaterial.properties) {
		if(math::is_flag_set(prop.propertyFlags, Property::Flags::HideInEditor))
			continue;
		auto socketType = shadergraph::to_data_type(pragma::shadergraph::to_udm_type(prop.parameter.type));
		if(socketType == shadergraph::DataType::Invalid)
			continue;
		AddOutput(string::to_camel_case(prop.parameter.name), socketType);
	}
}
std::string ShaderMaterialNode::GetTextureVariableName(const shadergraph::OutputSocket &socket) const { return shader_material::ShaderMaterial::GetTextureUniformVariableName(socket.GetSocket().name); }
std::string ShaderMaterialNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const
{
	// TODO: Only write output var is output is set?
	// get_mat_x();
	std::ostringstream code;
	for(auto &prop : m_shaderMaterial.properties) {
		auto socketType = prop.parameter.type;
		if(socketType == shadergraph::DataType::Invalid)
			continue;
		auto socketName = string::to_camel_case(prop.parameter.name);
		if(!gn.IsOutputLinked(socketName))
			continue;
		auto *glslType = pragma::shadergraph::to_glsl_type(socketType);
		code << glslType << " ";
		code << gn.GetOutputVarName(socketName) << " = ";
		code << "get_mat_" << prop.parameter.name.c_str() << "();\n";
	}
	return code.str();
}

// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_material_texture;

using namespace pragma::rendering::shader_graph;
MaterialTextureNode::MaterialTextureNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_TEXTURE}
{
	AddInput(IN_TEXTURE, shadergraph::DataType::String, "");
	AddInput(IN_VECTOR, shadergraph::DataType::Vector, Vector3 {0.f, 0.f, 0.f}); // TODO: Make input only, don't allow writing manually

	AddOutput(OUT_COLOR, shadergraph::DataType::Color);
	AddOutput(OUT_ALPHA, shadergraph::DataType::Float);
}

std::string MaterialTextureNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	std::string uv;
	if(gn.IsInputLinked(IN_VECTOR))
		uv = gn.GetInputNameOrValue(IN_VECTOR);
	else
		uv = "vec3(get_vertex_uv(), 0.0)";

	auto prefix = gn.GetBaseVarName() + "_";
	auto *texInputSocket = gn.FindInputSocket(IN_TEXTURE);
	if(texInputSocket && texInputSocket->link) {
		std::string texName = string::to_snake_case(texInputSocket->link->GetSocket().name);
		if(texInputSocket->link->parent) {
			auto *dynTexNode = dynamic_cast<const InputParameterTextureNode *>(&texInputSocket->link->parent->node);
			if(dynTexNode) {
				std::string name;
				if(texInputSocket->link->parent->GetInputValue(BaseInputParameterNode::CONST_NAME, name))
					texName = name;
			}
		}
		code << "vec4 " << prefix << "texCol = fetch_" << texName << "(" << uv << ".xy);\n";
	}
	else
		code << "vec4 " << prefix << "texCol = vec4(1.0, 1.0, 1.0, 1.0);\n";

	code << gn.GetGlslOutputDeclaration(OUT_COLOR) << " = ";
	code << prefix << "texCol.rgb;\n";

	code << gn.GetGlslOutputDeclaration(OUT_ALPHA) << " = ";
	code << prefix << "texCol.a;\n";
	return code.str();
}

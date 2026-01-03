// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_image_texture;

using namespace pragma::rendering::shader_graph;

ImageTextureNode::ImageTextureNode(const std::string_view &type) : ImageTextureNodeBase {type, false}
{
	AddInput(IN_FILENAME, shadergraph::DataType::String, "");
	AddInput(IN_VECTOR, shadergraph::DataType::Vector, Vector3 {0.f, 0.f, 0.f}); // TODO: Make input only, don't allow writing manually

	AddOutput(OUT_COLOR, shadergraph::DataType::Color);
	AddOutput(OUT_ALPHA, shadergraph::DataType::Float);
}

std::string ImageTextureNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	std::string uv;
	if(gn.IsInputLinked(IN_VECTOR))
		uv = gn.GetInputNameOrValue(IN_VECTOR);
	else
		uv = "vec3(get_vertex_uv(), 0.0)";

	auto prefix = gn.GetBaseVarName() + "_";
	code << "vec4 " << prefix << "texCol = texture(" << GetTextureVariableName(gn) << ", " << uv << ".xy);\n";

	code << gn.GetGlslOutputDeclaration(OUT_COLOR) << " = ";
	code << prefix << "texCol.rgb;\n";

	code << gn.GetGlslOutputDeclaration(OUT_ALPHA) << " = ";
	code << prefix << "texCol.a;\n";
	return code.str();
}

//

ImageTextureNodeBase::ImageTextureNodeBase(const std::string_view &type, bool populateOutputsAndInputs) : Node {type, shadergraph::CATEGORY_TEXTURE}
{
	if(populateOutputsAndInputs) {
		AddInput(IN_FILENAME, shadergraph::DataType::String, "");

		AddOutput(OUT_TEXTURE, shadergraph::DataType::String);
	}

	AddModuleDependency("image_texture");
}

std::string ImageTextureNodeBase::GetTextureVariableName(const shadergraph::OutputSocket &socket) const { return GetTextureVariableName(*socket.parent); }

std::string ImageTextureNodeBase::GetTextureVariableName(const shadergraph::GraphNode &gn) const
{
	auto prefix = gn.GetBaseVarName() + "_";
	return prefix + "tex";
}

std::string ImageTextureNodeBase::DoEvaluateResourceDeclarations(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	//auto texName = GetTextureVariableName(gn);
	//auto upperTexName = texName;
	//pragma::string::to_upper(upperTexName);
	//code << "layout(LAYOUT_ID(TEST, " << upperTexName << ")) uniform sampler2D " << texName << ";\n";
	return code.str();
}

std::string ImageTextureNodeBase::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	return code.str();
}

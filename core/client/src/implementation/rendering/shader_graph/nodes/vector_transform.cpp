// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_vector_transform;

using namespace pragma::rendering::shader_graph;

VectorTransformNode::VectorTransformNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_VECTOR_MATH}
{
	AddSocketEnum<Type>(IN_TRANSFORM_TYPE, Type::Vector);
	AddSocketEnum<Space>(IN_CONVERT_FROM, Space::World);
	AddSocketEnum<Space>(IN_CONVERT_TO, Space::Object);
	AddInput(IN_VECTOR, shadergraph::DataType::Vector, Vector3 {0.f, 0.f, 0.f});
	AddOutput(OUT_VECTOR, shadergraph::DataType::Vector);

	AddModuleDependency("camera");
	AddModuleDependency("entity");
}

std::string VectorTransformNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const
{
	std::ostringstream code;

	auto vector = gn.GetInputNameOrValue(IN_VECTOR);

	auto type = *gn.GetConstantInputValue<Type>(IN_TRANSFORM_TYPE);
	auto convertFrom = *gn.GetConstantInputValue<Space>(IN_CONVERT_FROM);
	auto convertTo = *gn.GetConstantInputValue<Space>(IN_CONVERT_TO);

	if(convertFrom == convertTo) {
		code << gn.GetGlslOutputDeclaration(OUT_VECTOR) << " = " << vector << ";\n";
		return code.str();
	}

	auto prefix = gn.GetBaseVarName() + "_";
	std::string matName = prefix + "mat";
	code << "mat4 " << matName << " = ";
	switch(convertFrom) {
	case Space::Object:
		switch(convertTo) {
		case Space::World:
			code << "get_model_matrix()";
			break;
		case Space::Camera:
			code << "get_view_matrix() *get_model_matrix()";
			break;
		}
		break;
	case Space::Camera:
		switch(convertTo) {
		case Space::World:
			code << "inverse(get_view_matrix())";
			break;
		case Space::Object:
			code << "inverse(get_model_matrix()) *inverse(get_view_matrix())";
			break;
		}
		break;
	case Space::World:
		switch(convertTo) {
		case Space::Object:
			code << "inverse(get_model_matrix())";
			break;
		case Space::Camera:
			code << "get_view_matrix()";
			break;
		}
		break;
	}
	code << ";\n";

	code << gn.GetGlslOutputDeclaration(OUT_VECTOR) << " = ";
	switch(type) {
	case Type::Vector:
		// Vectors are transformed with the matrix, ignoring translation.
		code << "(" << matName << " * vec4(" << vector << ", 0.0)).xyz";
		break;

	case Type::Normal:
		// Normals require the inverse transpose of the matrix.
		code << "(transpose(inverse(mat3(" << matName << "))) * " << vector << ")";
		break;

	case Type::Point:
		// Points are fully transformed, including translation.
		code << "(" << matName << " * vec4(" << vector << ", 1.0)).xyz";
		break;
	}
	code << ";\n";
	return code.str();
}

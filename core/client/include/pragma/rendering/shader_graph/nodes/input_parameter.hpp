/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_BASE_SHADER_GRAPH_NODES_INPUT_PARAMETER_HPP__
#define __PRAGMA_BASE_SHADER_GRAPH_NODES_INPUT_PARAMETER_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <string_view>
#include <string>

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT BaseInputParameterNode : public pragma::shadergraph::Node {
	  public:
		enum class Scope : uint32_t {
			Global = 0,
			Object,
			Material,
		};

		static constexpr const char *CONST_NAME = "name";
		static constexpr const char *CONST_SCOPE = "scope";
		static constexpr const char *CONST_DEFAULT = "default";

		static constexpr const char *OUT_VALUE = "value";

		BaseInputParameterNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_INPUT_PARAMETER}
		{
			AddInput(CONST_NAME, pragma::shadergraph::DataType::String, "");
			AddSocketEnum<Scope>(CONST_SCOPE, Scope::Global);

			AddModuleDependency("input_data");
		}
		virtual pragma::shadergraph::DataType GetParameterType() const = 0;
	};

	template<typename T>
	class DLLCLIENT InputParameterNode : public BaseInputParameterNode {
	  public:
		InputParameterNode(const std::string_view &type) : BaseInputParameterNode {type}
		{
			AddInput(CONST_DEFAULT, GetParameterType(), T {});
			AddOutput(OUT_VALUE, GetParameterType());
		}
		virtual pragma::shadergraph::DataType GetParameterType() const override { return pragma::shadergraph::to_data_type(udm::type_to_enum<T>()); }
		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const override
		{
			std::ostringstream code;

			std::string name;
			gn.GetInputValue<std::string>(CONST_NAME, name);

			code << gn.GetGlslOutputDeclaration(OUT_VALUE) << " = ";
			// TODO: Check if name exists in global input data
			if(!name.empty())
				code << "u_material.material." << name << ";\n";
			else
				code << "0.0;\n";

			return code.str();
		}
	};

	class DLLCLIENT InputParameterFloatNode : public InputParameterNode<float> {
	  public:
		static constexpr const char *CONST_MIN = "min";
		static constexpr const char *CONST_MAX = "max";
		static constexpr const char *CONST_STEP_SIZE = "stepSize";

		InputParameterFloatNode(const std::string_view &type) : InputParameterNode<float> {type}
		{
			AddInput(CONST_MIN, pragma::shadergraph::DataType::Float, 0.f);
			AddInput(CONST_MAX, pragma::shadergraph::DataType::Float, 1.f);
			AddInput(CONST_STEP_SIZE, pragma::shadergraph::DataType::Float, 0.1f);
		}
	};

	using InputParameterBooleanNode = InputParameterNode<udm::Boolean>;
	using InputParameterIntNode = InputParameterNode<udm::Int32>;
	using InputParameterUIntNode = InputParameterNode<udm::UInt32>;
	using InputParameterUInt16Node = InputParameterNode<udm::UInt16>;
	using InputParameterColorNode = InputParameterNode<udm::Vector3>;
	using InputParameterVectorNode = InputParameterNode<udm::Vector3>;
	using InputParameterVector4Node = InputParameterNode<udm::Vector4>;
	using InputParameterPointNode = InputParameterNode<udm::Vector3>;
	using InputParameterNormalNode = InputParameterNode<udm::Vector3>;
	using InputParameterPoint2Node = InputParameterNode<udm::Vector2>;
	using InputParameterStringNode = InputParameterNode<udm::String>;
	using InputParameterTransformNode = InputParameterNode<udm::Mat4>;
};

#endif

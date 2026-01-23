// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_input_parameter;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT BaseInputParameterNode : public shadergraph::Node {
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

		BaseInputParameterNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_INPUT_PARAMETER}
		{
			AddSocket(CONST_NAME, shadergraph::DataType::String, "");
			AddSocketEnum<Scope>(CONST_SCOPE, Scope::Global);

			AddModuleDependency("input_data");
		}
		virtual shadergraph::DataType GetParameterType() const = 0;
	};

	class DLLCLIENT InputParameterTextureNode : public BaseInputParameterNode {
	  public:
		enum class ColorSpace : uint8_t {
			Srgb = 0,
			Linear,
		};

		enum class ImageType : uint8_t {
			e2D = 0,
			Cube,
		};

		static constexpr const char *CONST_DEFAULT_TEXTURE = "defaultTexture";
		static constexpr const char *CONST_COLOR_SPACE = "colorSpace";
		static constexpr const char *CONST_IMAGE_TYPE = "imageType";

		static constexpr const char *OUT_TEXTURE = "texture";

		InputParameterTextureNode(const std::string_view &type) : BaseInputParameterNode {type}
		{
			AddSocket(CONST_DEFAULT_TEXTURE, shadergraph::DataType::String, "white");
			AddSocketEnum<ColorSpace>(CONST_COLOR_SPACE, ColorSpace::Srgb);
			AddSocketEnum<ImageType>(CONST_IMAGE_TYPE, ImageType::e2D);

			AddOutput(OUT_TEXTURE, shadergraph::DataType::String);
		}
		virtual shadergraph::DataType GetParameterType() const override { return shadergraph::DataType::String; }
		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const override
		{
			std::ostringstream code;
			return code.str();
		}
	};

	template<typename T>
	class DLLCLIENT InputParameterNode : public BaseInputParameterNode {
	  public:
		InputParameterNode(const std::string_view &type) : BaseInputParameterNode {type}
		{
			AddSocket(CONST_DEFAULT, GetParameterType(), T {});
			AddOutput(OUT_VALUE, GetParameterType());
		}
		virtual shadergraph::DataType GetParameterType() const override { return shadergraph::to_data_type(udm::type_to_enum<T>()); }
		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const override
		{
			std::ostringstream code;

			std::string name;
			gn.GetInputValue<std::string>(CONST_NAME, name);

			code << gn.GetGlslOutputDeclaration(OUT_VALUE) << " = ";
			// TODO: Check if name exists in global input data
			if(!name.empty())
				code << "u_material.material." << name << ";\n";
			else {
				auto type = GetParameterType();
				shadergraph::visit(type, [&code](auto tag) {
					using TValue = typename decltype(tag)::type;
					if constexpr(!std::is_same_v<TValue, udm::String>) {
						TValue value {};
						code << shadergraph::to_glsl_value(value) << ";\n";
					}
				});
			}

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
			AddSocket(CONST_MIN, shadergraph::DataType::Float, 0.f);
			AddSocket(CONST_MAX, shadergraph::DataType::Float, 1.f);
			AddSocket(CONST_STEP_SIZE, shadergraph::DataType::Float, 0.1f);
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

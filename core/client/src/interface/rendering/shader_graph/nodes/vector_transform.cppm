// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_vector_transform;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT VectorTransformNode : public shadergraph::Node {
	  public:
		enum class Type : uint8_t {
			Vector = 0,
			Point,
			Normal,
		};

		enum class Space : uint8_t {
			World = 0,
			Object,
			Camera,
		};

		static constexpr const char *IN_TRANSFORM_TYPE = "transformType";
		static constexpr const char *IN_CONVERT_FROM = "convertForm";
		static constexpr const char *IN_CONVERT_TO = "convertTo";
		static constexpr const char *IN_VECTOR = "vector";

		static constexpr const char *OUT_VECTOR = "vector";

		VectorTransformNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};

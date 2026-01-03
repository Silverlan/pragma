// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_image_texture;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT ImageTextureNodeBase : public shadergraph::Node {
	  public:
		static constexpr const char *IN_FILENAME = "fileName";

		static constexpr const char *OUT_TEXTURE = "texture";

		ImageTextureNodeBase(const std::string_view &type, bool populateOutputsAndInputs = true);
		std::string GetTextureVariableName(const shadergraph::GraphNode &gn) const;
		std::string GetTextureVariableName(const shadergraph::OutputSocket &socket) const;

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
		virtual std::string DoEvaluateResourceDeclarations(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
	class DLLCLIENT ImageTextureNode : public ImageTextureNodeBase {
	  public:
		static constexpr const char *IN_VECTOR = "vector";

		static constexpr const char *OUT_COLOR = "color";
		static constexpr const char *OUT_ALPHA = "alpha";

		ImageTextureNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};

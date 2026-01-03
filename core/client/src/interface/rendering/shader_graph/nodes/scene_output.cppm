// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_scene_output;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT SceneOutputNode : public shadergraph::Node {
	  public:
		static constexpr const char *IN_COLOR = "color";
		static constexpr const char *IN_ALPHA = "alpha";
		static constexpr const char *IN_ALPHA_CUTOFF = "alphaCutoff";
		static constexpr const char *IN_BLOOM_FACTOR = "bloomFactor";
		static constexpr const char *IN_EMISSIVE_COLOR = "emissiveColor";
		static constexpr const char *CONST_ALPHA_MODE = "alphaMode";
		// TODO: Only allow one of these!
		SceneOutputNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};

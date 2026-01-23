// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_pbr;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT PbrNode : public shadergraph::Node {
	  public:
		static constexpr const char *IN_ALBEDO_COLOR = "albedoColor";
		static constexpr const char *IN_METALNESS = "metalness";
		static constexpr const char *IN_ROUGHNESS = "roughness";
		static constexpr const char *IN_AMBIENT_OCCLUSION = "ambientOcclusion";

		static constexpr const char *OUT_COLOR = "color";

		PbrNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};

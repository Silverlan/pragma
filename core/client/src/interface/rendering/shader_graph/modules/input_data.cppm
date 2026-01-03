// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.module_input_data;

export import :rendering.shader_graph.sg_module;
export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT InputDataModule : public ShaderGraphModule {
	  public:
		InputDataModule(ShaderGraph &shader);
		virtual ~InputDataModule() override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode) override;
		virtual void RecordBindScene(ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
	  private:
		prosper::DescriptorSetInfo m_globalInputDataDsInfo;

		std::unique_ptr<shadergraph::Graph> m_resolvedGraph;

		std::shared_ptr<prosper::IDescriptorSetGroup> m_globalInputDsg;
		std::vector<const shadergraph::GraphNode *> m_imageTextureNodes;
	};
};

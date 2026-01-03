// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_graph;

export import :rendering.shader_graph.sg_module;
export import :rendering.shaders.textured;
export import pragma.shadergraph;

export namespace pragma {
	class DLLCLIENT ShaderGraph : public ShaderGameWorldLightingPass {
	  public:
		ShaderGraph(prosper::IPrContext &context, const std::shared_ptr<shadergraph::Graph> &sg, const std::string &identifier, const std::string &fsShader);
		virtual ~ShaderGraph() override;

		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const override;
		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, material::CMaterial &mat) const override;
		virtual bool IsTranslucentPipeline(uint32_t pipelineIdx) const override;

		const shadergraph::Graph *GetGraph() const;
	  protected:
		std::shared_ptr<rendering::shader_material::ShaderMaterial> GenerateShaderMaterial();
		virtual void InitializeShaderMaterial() override;

		virtual void OnPipelinesInitialized() override;
		virtual void ClearShaderResources() override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeMaterialData(const material::CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, rendering::ShaderInputData &inOutMatData) override;
		virtual void UpdateRenderFlags(geometry::CModelSubMesh &mesh, SceneFlags &inOutFlags) override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode) override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo);

		std::shared_ptr<shadergraph::Graph> m_shaderGraph;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_defaultPbrDsg = nullptr;
		std::vector<std::unique_ptr<rendering::ShaderGraphModule>> m_modules;
		AlphaMode m_alphaMode = AlphaMode::Opaque;
	};
};

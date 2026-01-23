// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.sg_module;

export import :entities.components.rasterization_renderer;
export import :entities.components.scene;
export import :model.mesh;
export import :rendering.render_processor;
export import pragma.shadergraph;

export namespace pragma {
	class ShaderGraph;
};
export namespace pragma::rendering {
	class DLLCLIENT ShaderGraphModule {
	  public:
		ShaderGraphModule(ShaderGraph &shader) : m_shader(shader) {}
		virtual ~ShaderGraphModule() {}
		virtual void InitializeShaderResources() {}
		virtual void InitializeGfxPipelineDescriptorSets() = 0;
		virtual void GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode) {}
		virtual void UpdateRenderFlags(geometry::CModelSubMesh &mesh, ShaderGameWorld::SceneFlags &inOutFlags) {}
		virtual void RecordBindScene(ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const = 0;
		virtual void RecordBindEntity(ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const {}
		virtual void RecordBindMaterial(ShaderProcessor &shaderProcessor, material::CMaterial &mat) const {}
		void SetNodes(std::vector<shadergraph::GraphNode *> &&nodes) { m_nodes = std::move(nodes); }
	  protected:
		ShaderGraph &m_shader;
		std::vector<shadergraph::GraphNode *> m_nodes;
	};

	class DLLCLIENT ShaderGraphModuleManager {
	  public:
		using Factory = std::function<std::unique_ptr<ShaderGraphModule>(ShaderGraph &shader)>;
		ShaderGraphModuleManager() {}
		void RegisterFactory(const std::string &name, const Factory &factory);
		std::unique_ptr<ShaderGraphModule> CreateModule(const std::string &name, ShaderGraph &shader, std::vector<shadergraph::GraphNode *> &&nodes) const;
		const std::unordered_map<std::string, Factory> &GetFactories() const { return m_factories; }
	  private:
		std::unordered_map<std::string, Factory> m_factories;
	};
}

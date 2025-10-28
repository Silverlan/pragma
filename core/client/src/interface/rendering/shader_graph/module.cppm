// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.shader_graph.module;

import :entities.components.rasterization_renderer;
import :entities.components.scene;
import :model.mesh;
import :rendering.render_processor;
import pragma.shadergraph;

namespace pragma {class ShaderGraph;};
export namespace pragma::rendering {
	class DLLCLIENT ShaderGraphModule {
	  public:
		ShaderGraphModule(ShaderGraph &shader) : m_shader(shader) {}
		virtual ~ShaderGraphModule() {}
		virtual void InitializeShaderResources() {}
		virtual void InitializeGfxPipelineDescriptorSets() = 0;
		virtual void GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode) {}
		virtual void UpdateRenderFlags(CModelSubMesh &mesh, ShaderGameWorld::SceneFlags &inOutFlags) {}
		virtual void RecordBindScene(ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const = 0;
		virtual void RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const {}
		virtual void RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const {}
		void SetNodes(std::vector<pragma::shadergraph::GraphNode *> &&nodes) { m_nodes = std::move(nodes); }
	  protected:
		ShaderGraph &m_shader;
		std::vector<pragma::shadergraph::GraphNode *> m_nodes;
	};

	class DLLCLIENT ShaderGraphModuleManager {
	  public:
		using Factory = std::function<std::unique_ptr<ShaderGraphModule>(ShaderGraph &shader)>;
		ShaderGraphModuleManager() {}
		void RegisterFactory(const std::string &name, const Factory &factory);
		std::unique_ptr<ShaderGraphModule> CreateModule(const std::string &name, ShaderGraph &shader, std::vector<pragma::shadergraph::GraphNode *> &&nodes) const;
		const std::unordered_map<std::string, Factory> &GetFactories() const { return m_factories; }
	  private:
		std::unordered_map<std::string, Factory> m_factories;
	};
}


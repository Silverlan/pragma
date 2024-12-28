/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SHADER_GRAPH_MODULE_HPP__
#define __SHADER_GRAPH_MODULE_HPP__

#include <pragma/clientdefinitions.h>
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"

import pragma.shadergraph;

namespace prosper {
	class Shader;
}

namespace pragma {
	class CSceneComponent;
	class CRasterizationRendererComponent;
	class ShaderGraph;
};

class CModelSubMesh;
namespace pragma::rendering {
	class ShaderProcessor;
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

#endif

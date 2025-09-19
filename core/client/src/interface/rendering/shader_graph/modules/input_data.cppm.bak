// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shader_graph/module.hpp"
#include "pragma/rendering/global_shader_input_manager.hpp"
#include "buffers/prosper_buffer.hpp"

export module pragma.client.rendering.shader_graph:module_input_data;

import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT InputDataModule : public pragma::rendering::ShaderGraphModule {
	  public:
		InputDataModule(ShaderGraph &shader);
		virtual ~InputDataModule() override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode) override;
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
	  private:
		prosper::DescriptorSetInfo m_globalInputDataDsInfo;

		std::unique_ptr<pragma::shadergraph::Graph> m_resolvedGraph;

		std::shared_ptr<prosper::IDescriptorSetGroup> m_globalInputDsg;
		std::vector<const pragma::shadergraph::GraphNode *> m_imageTextureNodes;
	};
};

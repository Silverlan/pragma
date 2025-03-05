/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __C_SHADER_GRAPH_HPP__
#define __C_SHADER_GRAPH_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

import pragma.shadergraph;

class Texture;
namespace pragma {
	namespace rendering {
		class ShaderGraphModule;
		namespace shader_material {
			struct ShaderMaterial;
		};
	};
	class DLLCLIENT ShaderGraph : public ShaderGameWorldLightingPass {
	  public:
		ShaderGraph(prosper::IPrContext &context, const std::shared_ptr<pragma::shadergraph::Graph> &sg, const std::string &identifier, const std::string &fsShader);
		virtual ~ShaderGraph() override;

		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const override;
		virtual bool IsTranslucentPipeline(uint32_t pipelineIdx) const override;

		const pragma::shadergraph::Graph *GetGraph() const;
	  protected:
		std::shared_ptr<pragma::rendering::shader_material::ShaderMaterial> GenerateShaderMaterial();
		virtual void InitializeShaderMaterial() override;

		using ShaderGameWorldLightingPass::RecordDraw;
		virtual void OnPipelinesInitialized() override;
		virtual void ClearShaderResources() override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeMaterialData(const CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::ShaderInputData &inOutMatData) override;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags) override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode) override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo);

		std::shared_ptr<pragma::shadergraph::Graph> m_shaderGraph;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_defaultPbrDsg = nullptr;
		std::vector<std::unique_ptr<rendering::ShaderGraphModule>> m_modules;
		AlphaMode m_alphaMode = AlphaMode::Opaque;
	};
};

#endif

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __C_SHADER_GRAPH_HPP__
#define __C_SHADER_GRAPH_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

class Texture;
namespace pragma {
	namespace rendering {
		class ShaderGraphModule;
	};
	class DLLCLIENT ShaderGraph : public ShaderGameWorldLightingPass {
	  public:
		ShaderGraph(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader);
		virtual ~ShaderGraph() override;

		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
	  protected:
		using ShaderGameWorldLightingPass::RecordDraw;
		virtual void OnPipelinesInitialized() override;
		virtual void ClearShaderResources() override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeMaterialData(const CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::shader_material::ShaderMaterialData &inOutMatData) override;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags) override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo);

		std::shared_ptr<prosper::IDescriptorSetGroup> m_defaultPbrDsg = nullptr;
		std::vector<std::unique_ptr<rendering::ShaderGraphModule>> m_modules;
	};
};

#endif

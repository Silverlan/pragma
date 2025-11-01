// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:rendering.shader_graph.module_image_texture;

import :entities.components.scene;
import :entities.components.rasterization_renderer;
import :rendering.render_processor;
export import :rendering.shader_graph.sg_module;
import :rendering.shaders.world_graph;
export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT ImageTextureModule : public pragma::rendering::ShaderGraphModule {
	  public:
		enum class PBRBinding : uint32_t {
			IrradianceMap = 0u,
			PrefilterMap,
			BRDFMap,

			Count
		};
		ImageTextureModule(ShaderGraph &shader);
		virtual ~ImageTextureModule() override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
	  private:
		prosper::DescriptorSetInfo m_descSetInfo;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dsg;
	};
};

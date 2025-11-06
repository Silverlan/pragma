// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:rendering.shader_graph.module_toon;

export import :rendering.shader_graph.sg_module;
export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT ToonModule : public pragma::rendering::ShaderGraphModule {
	  public:
		/*enum class PBRBinding : uint32_t {
			IrradianceMap = 0u,
			PrefilterMap,
			BRDFMap,

			Count
		};*/
		ToonModule(ShaderGraph &shader);
		virtual ~ToonModule() override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
	  private:
		prosper::DescriptorSetInfo m_pbrDescSetInfo;
		static std::shared_ptr<prosper::IDescriptorSetGroup> g_defaultPbrDsg;
		static size_t g_instanceCount;
	};
};

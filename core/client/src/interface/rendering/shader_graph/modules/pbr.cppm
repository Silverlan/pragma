// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.module_pbr;

export import :rendering.shader_graph.sg_module;
export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT PbrModule : public ShaderGraphModule {
	  public:
		enum class PBRBinding : uint32_t {
			IrradianceMap = 0u,
			PrefilterMap,
			BRDFMap,

			Count
		};
		PbrModule(ShaderGraph &shader);
		virtual ~PbrModule() override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void RecordBindScene(ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
		prosper::IDescriptorSet *GetReflectionProbeDescriptorSet(const CSceneComponent &scene, float &outIblStrength, ShaderGameWorld::SceneFlags &inOutSceneFlags) const;
		prosper::IDescriptorSet &GetDefaultPbrDescriptorSet() const;
	  private:
		prosper::DescriptorSetInfo m_pbrDescSetInfo;
		static std::shared_ptr<prosper::IDescriptorSetGroup> g_defaultPbrDsg;
		static size_t g_instanceCount;
	};
};

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_water;

export import :rendering.shaders.textured;

export namespace pragma {
	class DLLCLIENT ShaderWater : public ShaderGameWorldLightingPass {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_WATER;
		enum class MaterialBinding : uint32_t {
			DuDvMap = 0u,
			NormalMap,

			Count
		};

		enum class WaterBinding : uint32_t {
			ReflectionMap = 0u,
			RefractionMap,
			RefractionDepth,
			WaterSettings,
			WaterFog,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			float waterFogIntensity;
			uint32_t enableReflection;
		};
#pragma pack(pop)

		ShaderWater(prosper::IPrContext &context, const std::string &identifier);
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat) override;

		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
		virtual GameShaderSpecializationConstantFlag GetBaseSpecializationFlags() const override;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const;
	  private:
		bool UpdateBindFogDensity();
	};
};

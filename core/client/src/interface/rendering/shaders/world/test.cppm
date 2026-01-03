// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_test;

export import :rendering.shaders.textured;

export namespace pragma {
	class DLLCLIENT ShaderTest : public ShaderGameWorldLightingPass {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PBR;

		enum class MaterialBinding : uint32_t {
			MaterialSettings = math::to_integral(ShaderGameWorldLightingPass::MaterialBinding::MaterialSettings),
			AlbedoMap,
			NormalMap,
			RMAMap,
			EmissionMap,
			ParallaxMap,
			WrinkleStretchMap,
			WrinkleCompressMap,
			ExponentMap,

			Count
		};

		enum class PBRBinding : uint32_t {
			IrradianceMap = 0u,
			PrefilterMap,
			BRDFMap,

			Count
		};

		ShaderTest(prosper::IPrContext &context, const std::string &identifier);
		ShaderTest(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");

		void DrawTest(prosper::IBuffer &buf, prosper::IBuffer &ibuf, uint32_t count);
		void SetForceNonIBLMode(bool b);
	  protected:
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void UpdateRenderFlags(geometry::CModelSubMesh &mesh, SceneFlags &inOutFlags) override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;

		SceneFlags m_extRenderFlags = SceneFlags::None;
		bool m_bNonIBLMode = false;
		Mat4 m_testMvp;
	};
};

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.prepass;

export import :rendering.shaders.scene;

export namespace pragma {
	class DLLCLIENT ShaderPrepassBase : public ShaderGameWorld {
	  public:
		enum class MaterialBinding : uint32_t {
			AlbedoMap = 0u,

			Count
		};
		static prosper::util::RenderPassCreateInfo::AttachmentInfo get_depth_render_pass_attachment_info(prosper::SampleCountFlags sampleCount);

		static ShaderGraphics::VertexBinding VERTEX_BINDING_RENDER_BUFFER_INDEX;
		static VertexAttribute VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX;

		static ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT;
		static VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_ID;
		static VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT;

		static ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT_EXT;
		static VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID;
		static VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT;

		static ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static VertexAttribute VERTEX_ATTRIBUTE_UV;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;

#pragma pack(push, 1)
		struct PushConstants : public ScenePushConstants {
			float alphaCutoff;

			void Initialize()
			{
				ScenePushConstants::Initialize();
				alphaCutoff = 0.5f;
			}
		};
#pragma pack(pop)

		ShaderPrepassBase(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader);
		ShaderPrepassBase(prosper::IPrContext &context, const std::string &identifier);

		virtual GameShaderType GetPassType() const override { return GameShaderType::DepthPrepass; }
		virtual bool IsDepthPrepassShader() const override { return true; }
		virtual uint32_t GetSceneDescriptorSetIndex() const override;

		//
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const override;
		virtual void RecordAlphaCutoff(rendering::ShaderProcessor &shaderProcessor, float alphaCutoff) const override;
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, material::CMaterial &mat) const override;
	  protected:
		virtual void OnPipelinesInitialized() override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;

		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
	  private:
		// These are unused
		virtual uint32_t GetLightDescriptorSetIndex() const { return std::numeric_limits<uint32_t>::max(); }
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dummyMaterialDsg = nullptr;
		std::optional<float> m_alphaCutoff {};
	};

	//////////////////

	class DLLCLIENT ShaderPrepass : public ShaderPrepassBase {
	  public:
		static VertexAttribute VERTEX_ATTRIBUTE_NORMAL;

		static prosper::Format RENDER_PASS_NORMAL_FORMAT;
		static prosper::util::RenderPassCreateInfo::AttachmentInfo get_normal_render_pass_attachment_info(prosper::SampleCountFlags sampleCount);

		enum class Pipeline : uint32_t {
			Opaque = 0u,
			AlphaTest,
			AnimatedOpaque,
			AnimatedAlphaTest,

			Count
		};

		enum class SpecializationConstant : uint32_t {
			EnableAlphaTest = 0u,
			EnableNormalOutput,
			EnableAnimation,
			EnableMorphTargetAnimation,
			EnableExtendedVertexWeights,

			Count
		};

		ShaderPrepass(prosper::IPrContext &context, const std::string &identifier);
		virtual uint32_t GetPassPipelineIndexStartOffset(rendering::PassType passType) const override;
	  protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

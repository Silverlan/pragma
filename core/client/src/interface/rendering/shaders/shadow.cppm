// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.shadow;

export import :rendering.shaders.scene;

export namespace pragma {
	class CSceneComponent;
	class CLightComponent;
};
export namespace pragma {
	class DLLCLIENT ShaderShadow : public ShaderGameWorld {
	  public:
		static prosper::Format RENDER_PASS_DEPTH_FORMAT;

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

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;

		enum class Pipeline : uint32_t {
			WithMorphTargetAnimations,
			Default = WithMorphTargetAnimations,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			void Initialize()
			{
				depthMVP = umat::identity();
				lightPos = {};
				flags = SceneFlags::None;
			}
			Mat4 depthMVP;
			Vector4 lightPos; // 4th component stores the distance
			SceneFlags flags;
			float alphaCutoff;
			uint32_t vertexAnimInfo;
		};
#pragma pack(pop)

		ShaderShadow(prosper::IPrContext &context, const std::string &identifier);
		ShaderShadow(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader);

		virtual GameShaderType GetPassType() const override { return GameShaderType::ShadowPass; }

		//
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const override;
		virtual void RecordSceneFlags(rendering::ShaderProcessor &shaderProcessor, SceneFlags sceneFlags) const override;
		virtual void RecordBindLight(rendering::ShaderProcessor &shaderProcessor, CLightComponent &light, uint32_t layerId) const override;
		virtual void RecordAlphaCutoff(rendering::ShaderProcessor &shaderProcessor, float alphaCutoff) const override;
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, material::CMaterial &mat) const override;
		virtual void RecordClipPlane(rendering::ShaderProcessor &shaderProcessor, const Vector4 &clipPlane) const override {}
		virtual void RecordDrawOrigin(rendering::ShaderProcessor &shaderProcessor, const Vector4 &drawOrigin) const override {}
		virtual void RecordDepthBias(rendering::ShaderProcessor &shaderProcessor, const Vector2 &depthBias) const override {}
		virtual void RecordVertexAnimationOffset(rendering::ShaderProcessor &shaderProcessor, uint32_t vertexAnimationOffset) const override;
	  protected:
		bool BindEntityDepthMatrix(const Mat4 &depthMVP);
		virtual void OnPipelinesInitialized() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual uint32_t GetSceneDescriptorSetIndex() const override;
	  private:
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
	};

	class DLLCLIENT ShaderShadowTransparent : public ShaderShadow {
	  public:
		//bool BindMaterial(material::CMaterial &mat);
	};

	//////////////////

	class DLLCLIENT ShaderShadowSpot : public ShaderShadow {
	  public:
		ShaderShadowSpot(prosper::IPrContext &context, const std::string &identifier);
	};

	//////////////////

	class DLLCLIENT ShaderShadowCSM : public ShaderShadow {
	  public:
		ShaderShadowCSM(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};

	//////////////////

	class DLLCLIENT ShaderShadowCSMTransparent : public ShaderShadowCSM {
	  public:
		//bool BindMaterial(material::CMaterial &mat);
	};
};

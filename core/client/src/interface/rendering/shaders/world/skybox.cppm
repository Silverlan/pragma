// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_skybox;

export import :rendering.shaders.textured;

export namespace pragma {
	class DLLCLIENT ShaderSkybox : public ShaderGameWorldLightingPass {
	  public:
		static ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;

#pragma pack(push, 1)
		struct PushConstants {
			Vector4 skyAngles; // Axis angle (radians)
		};
#pragma pack(pop)

		ShaderSkybox(prosper::IPrContext &context, const std::string &identifier);
		virtual bool GetRenderBufferTargets(geometry::CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const override;
		virtual GameShaderType GetPassType() const override { return GameShaderType::SkyPass; }

		//
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const override;
		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
		virtual void RecordSceneFlags(rendering::ShaderProcessor &shaderProcessor, SceneFlags sceneFlags) const override {}
		virtual void RecordClipPlane(rendering::ShaderProcessor &shaderProcessor, const Vector4 &clipPlane) const override {}
		virtual void RecordDrawOrigin(rendering::ShaderProcessor &shaderProcessor, const Vector4 &drawOrigin) const override {}
		virtual void RecordDepthBias(rendering::ShaderProcessor &shaderProcessor, const Vector2 &depthBias) const override {}
		virtual void RecordVertexAnimationOffset(rendering::ShaderProcessor &shaderProcessor, uint32_t vertexAnimationOffset) const override {}
		virtual bool IsDebugPrintEnabled() const override { return false; }
	  protected:
		ShaderSkybox(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader);
		virtual uint32_t GetRendererDescriptorSetIndex() const override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual uint32_t GetSceneDescriptorSetIndex() const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		EulerAngles m_skyAngles = {};
	};

	//////////////

	class DLLCLIENT ShaderSkyboxEquirect : public ShaderSkybox {
	  public:
		ShaderSkyboxEquirect(prosper::IPrContext &context, const std::string &identifier);
	};
};

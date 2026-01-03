// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle_blob;

export import :rendering.shaders.particle_2d_base;

export namespace pragma {
	namespace rendering::shader_material {
		struct ShaderMaterial;
	};
	class DLLCLIENT ShaderParticleBlob : public ShaderParticle2DBase {
	  public:
		enum class DebugMode : uint8_t { None = 0, EyeDir, SurfaceNormal, FlatColor };

		static const uint32_t MAX_BLOB_NEIGHBORS = 8;

		static VertexBinding VERTEX_BINDING_BLOB_NEIGHBORS;
		static ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BLOB_NEIGHBORS;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PBR;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PARTICLE_DATA;

		ShaderParticleBlob(prosper::IPrContext &context, const std::string &identifier);
		virtual uint32_t GetSceneDescriptorSetIndex() const override;
		bool RecordDraw(prosper::ShaderBindState &bindState, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const ecs::CParticleSystemComponent &ps, pts::ParticleOrientationType orientationType, pts::ParticleRenderFlags renderFlags,
		  prosper::IBuffer &blobIndexBuffer, prosper::IDescriptorSet &dsParticles, uint32_t particleBufferOffset);
		virtual bool RecordBindScene(prosper::ICommandBuffer &cmd, const prosper::IShaderPipelineLayout &layout, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
		  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsShadows) const override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat) override;
	  protected:
		virtual void GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual bool RecordParticleMaterial(prosper::ShaderBindState &bindState, const CRasterizationRendererComponent &renderer, const ecs::CParticleSystemComponent &ps) const override;
		std::shared_ptr<rendering::shader_material::ShaderMaterial> m_shaderMaterial;
		std::unique_ptr<prosper::DescriptorSetInfo> m_materialDescSetInfo;
	};
};

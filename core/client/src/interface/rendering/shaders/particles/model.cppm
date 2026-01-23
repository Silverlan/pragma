// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle_model;

export import :rendering.shaders.particle_base;
export import :rendering.shaders.textured;

export namespace pragma {
	class DLLCLIENT ShaderParticleModel : public ShaderGameWorldLightingPass, public ShaderParticleBase {
	  public:
		static ShaderGraphics::VertexBinding VERTEX_BINDING_PARTICLE;
		static VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static VertexAttribute VERTEX_ATTRIBUTE_RADIUS;
		static VertexAttribute VERTEX_ATTRIBUTE_PREVPOS;
		static VertexAttribute VERTEX_ATTRIBUTE_AGE;
		static VertexAttribute VERTEX_ATTRIBUTE_COLOR;

		static ShaderGraphics::VertexBinding VERTEX_BINDING_ROTATION;
		static VertexAttribute VERTEX_ATTRIBUTE_ROTATION;

		static ShaderGraphics::VertexBinding VERTEX_BINDING_ANIMATION_START;
		static VertexAttribute VERTEX_ATTRIBUTE_ANIMATION_START;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_ANIMATION;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_BONE_MATRICES;

#pragma pack(push, 1)
		struct PushConstants {
			uint32_t renderFlags;
			uint32_t alphaMode;
		};
#pragma pack(pop)

		ShaderParticleModel(prosper::IPrContext &context, const std::string &identifier);
		bool RecordParticleBuffers(prosper::ShaderBindState &bindState, prosper::IBuffer &particleBuffer, prosper::IBuffer &rotBuffer, prosper::IBuffer &animStartBuffer);
		bool Draw(geometry::CModelSubMesh &mesh, uint32_t numInstances, uint32_t firstInstance = 0u);
		bool RecordParticleSystem(prosper::ShaderBindState &bindState, ecs::CParticleSystemComponent &pSys) const;

		bool RecordBeginDraw(prosper::ShaderBindState &bindState, const Vector4 &clipPlane, ecs::CParticleSystemComponent &pSys, const Vector4 &drawOrigin = {0.f, 0.f, 0.f, 1.f}, RecordFlags recordFlags = RecordFlags::RenderPassTargetAsViewportAndScissor) const;
	  protected:
		virtual prosper::DescriptorSetInfo &GetAnimationDescriptorSetInfo() const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual bool ShouldInitializePipeline(uint32_t pipelineIdx) override;
	  private:
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dummyAnimDescSetGroup = nullptr;
	};
};

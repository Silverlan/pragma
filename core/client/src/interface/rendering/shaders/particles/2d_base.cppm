// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle_2d_base;

export import :rendering.shaders.particle_base;
export import :rendering.shaders.scene;
export import pragma.shared;

export namespace pragma {
	class CRasterizationRendererComponent;
	class CSceneComponent;
	namespace ecs {
		class CParticleSystemComponent;
	};
}
export namespace pragma {
	class DLLCLIENT ShaderParticle2DBase : public ShaderSceneLit, public ShaderParticleBase {
	  public:
		static VertexBinding VERTEX_BINDING_PARTICLE;
		static VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static VertexAttribute VERTEX_ATTRIBUTE_RADIUS;
		static VertexAttribute VERTEX_ATTRIBUTE_PREVPOS;
		static VertexAttribute VERTEX_ATTRIBUTE_AGE;
		static VertexAttribute VERTEX_ATTRIBUTE_COLOR;
		static VertexAttribute VERTEX_ATTRIBUTE_ROTATION;
		static VertexAttribute VERTEX_ATTRIBUTE_LENGTH_YAW;

		static VertexBinding VERTEX_BINDING_ANIMATION_START;
		static VertexAttribute VERTEX_ATTRIBUTE_ANIMATION_FRAME_INDICES;
		static VertexAttribute VERTEX_ATTRIBUTE_ANIMATION_INTERP_FACTOR;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_MAP;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_ANIMATION;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;

		static constexpr auto VERTEX_COUNT = 6u;
		static constexpr auto TRIANGLE_COUNT = 2u;
		static std::array<Vector2, 4> GetQuadVertexPositions();
		static Vector2 GetVertexUV(uint32_t vertIdx);
		enum class VertexAttribute : uint32_t {
			Vertex = 0u,
			Particle,
			Color,
			Rotation,
			AnimationStart,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			Vector4 colorFactor;
			Vector3 camRightWs;
			int32_t orientation;
			Vector3 camUpWs;
			float nearZ;
			Vector3 camPos;
			float farZ;
			uint32_t viewportSize; // First 16 bits = width, second 16 bits = height
			uint32_t renderFlags;
			uint32_t alphaMode;
			float time;
		};
#pragma pack(pop)

		ShaderParticle2DBase(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");
		virtual bool RecordDraw(prosper::ShaderBindState &bindState, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const ecs::CParticleSystemComponent &ps, pts::ParticleOrientationType orientationType, pts::ParticleRenderFlags renderFlags);
		std::optional<uint32_t> RecordBeginDraw(prosper::ShaderBindState &bindState, ecs::CParticleSystemComponent &pSys, pts::ParticleRenderFlags renderFlags, RecordFlags recordFlags = RecordFlags::RenderPassTargetAsViewportAndScissor);
		virtual bool RecordBindScene(prosper::ICommandBuffer &cmd, const prosper::IShaderPipelineLayout &layout, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
		  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsShadows) const;
		virtual uint32_t GetSceneDescriptorSetIndex() const;

		void GetParticleSystemOrientationInfo(const Mat4 &matrix, const ecs::CParticleSystemComponent &particle, pts::ParticleOrientationType orientationType, Vector3 &up, Vector3 &right, float &nearZ, float &farZ, const material::Material *material = nullptr, float camNearZ = 0.f,
		  float camFarZ = 0.f) const;

		Vector3 CalcVertexPosition(const ecs::CParticleSystemComponent &ptc, uint32_t ptIdx, uint32_t absVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ) const;

		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat) override;
	  protected:
		virtual Vector3 DoCalcVertexPosition(const ecs::CParticleSystemComponent &ptc, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ) const;

		virtual prosper::DescriptorSetInfo &GetAnimationDescriptorSetInfo() const override;
		virtual bool RecordParticleMaterial(prosper::ShaderBindState &bindState, const CRasterizationRendererComponent &renderer, const ecs::CParticleSystemComponent &ps) const;

		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual bool ShouldInitializePipeline(uint32_t pipelineIdx) override;

		void RegisterDefaultGfxPipelineVertexAttributes();
		void RegisterDefaultGfxPipelinePushConstantRanges();
		void RegisterDefaultGfxPipelineDescriptorSetGroups();

		void GetParticleSystemOrientationInfo(const Mat4 &matrix, const ecs::CParticleSystemComponent &particle, pts::ParticleOrientationType orientationType, Vector3 &up, Vector3 &right, float &nearZ, float &farZ, const material::Material *material = nullptr,
		  const BaseEnvCameraComponent *cam = nullptr) const;
		virtual void GetParticleSystemOrientationInfo(const Mat4 &matrix, const ecs::CParticleSystemComponent &particle, Vector3 &up, Vector3 &right, float &nearZ, float &farZ, const material::Material *material = nullptr, const BaseEnvCameraComponent *cam = nullptr) const;
	};
};

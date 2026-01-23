// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle;

export import :rendering.shaders.particle_2d_base;

export namespace pragma {
	class DLLCLIENT ShaderParticle : public ShaderParticle2DBase {
	  public:
		ShaderParticle(prosper::IPrContext &context, const std::string &identifier);
	};

	class DLLCLIENT ShaderParticleRotational : public ShaderParticle2DBase {
	  public:
		static VertexBinding VERTEX_BINDING_WORLD_ROTATION;
		static ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_WORLD_ROTATION;

		enum class VertexAttribute : uint32_t { WorldRotation = math::to_integral(ShaderParticle2DBase::VertexAttribute::Count) };

		bool RecordWorldRotationBuffer(prosper::ShaderBindState &bindState, prosper::IBuffer &buffer) const;
		ShaderParticleRotational(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void GetParticleSystemOrientationInfo(const Mat4 &matrix, const ecs::CParticleSystemComponent &particle, Vector3 &up, Vector3 &right, float &nearZ, float &farZ, const material::Material *material = nullptr, const BaseEnvCameraComponent *cam = nullptr) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

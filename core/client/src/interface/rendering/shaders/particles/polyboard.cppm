// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle_polyboard;

export import :rendering.shaders.particle_2d_base;

export namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT ShaderParticlePolyboard : public ShaderParticle2DBase {
	  public:
		static VertexBinding VERTEX_BINDING_VERTEX;
		static ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_VERTEX;
		static ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;

		ShaderParticlePolyboard(prosper::IPrContext &context, const std::string &identifier);

#pragma pack(push, 1)
		struct GeometryPushConstants {
			Vector3 posCam;
			float radius;
			float curvature;
		};
		struct FragmentPushConstants {
			int32_t renderFlags;
			float viewportW;
			float viewportH;
		};
#pragma pack(pop)
		bool Draw(CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const ecs::CParticleSystemComponent &ps, prosper::IBuffer &vertexBuffer, prosper::IBuffer &indexBuffer, uint32_t numIndices, float radius, float curvature);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	  private:
		bool Draw(CSceneComponent &scene, const ecs::CParticleSystemComponent &ps, bool bloom) = delete;
	};
};

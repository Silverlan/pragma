// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_PARTICLE_POLYBOARD_HPP__
#define __C_SHADER_PARTICLE_POLYBOARD_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT ShaderParticlePolyboard : public ShaderParticle2DBase {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;

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
		bool Draw(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const pragma::CParticleSystemComponent &ps, prosper::IBuffer &vertexBuffer, prosper::IBuffer &indexBuffer, uint32_t numIndices, float radius, float curvature);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	  private:
		bool Draw(pragma::CSceneComponent &scene, const pragma::CParticleSystemComponent &ps, bool bloom) = delete;
	};
};

#endif

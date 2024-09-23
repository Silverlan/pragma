/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PARTICLE_MODEL_HPP__
#define __C_SHADER_PARTICLE_MODEL_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_base.hpp"

namespace pragma {
	class DLLCLIENT ShaderParticleModel : public ShaderGameWorldLightingPass, public ShaderParticleBase {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_PARTICLE;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_RADIUS;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_PREVPOS;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_AGE;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ROTATION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ROTATION;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ANIMATION_START;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ANIMATION_START;

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
		bool Draw(CModelSubMesh &mesh, uint32_t numInstances, uint32_t firstInstance = 0u);
		bool RecordParticleSystem(prosper::ShaderBindState &bindState, pragma::CParticleSystemComponent &pSys) const;

		bool RecordBeginDraw(prosper::ShaderBindState &bindState, const Vector4 &clipPlane, pragma::CParticleSystemComponent &pSys, const Vector4 &drawOrigin = {0.f, 0.f, 0.f, 1.f},
		  ShaderScene::RecordFlags recordFlags = ShaderScene::RecordFlags::RenderPassTargetAsViewportAndScissor) const;
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

#endif

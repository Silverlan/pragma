/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_PARTICLE_2D_BASE_HPP__
#define __C_SHADER_PARTICLE_2D_BASE_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_base.hpp"
#include "pragma/rendering/c_alpha_mode.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"

namespace pragma
{
	namespace rendering {class RasterizationRenderer;};
	class DLLCLIENT ShaderParticle2DBase
		: public ShaderSceneLit,
		public ShaderParticleBase
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_VERTEX;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_PARTICLE;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_PARTICLE;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ROTATION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_LENGTH;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ANIMATION_START;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ANIMATION_START;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_MAP;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_ANIMATION;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CSM;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;

		enum class VertexAttribute : uint32_t
		{
			Vertex = 0u,
			Particle,
			Color,
			Rotation,
			AnimationStart,

			Count
		};

#pragma pack(push,1)
		struct PushConstants
		{
			Vector3 camRightWs;
			int32_t orientation;
			Vector3 camUpWs;
			float nearZ;
			Vector3 camPos;
			float farZ;
			uint32_t viewportSize; // First 16 bits = width, second 16 bits = height
			float texIntensity;
			uint32_t renderFlags;
			uint32_t alphaMode;
			float time;
		};
#pragma pack(pop)

		ShaderParticle2DBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		bool Draw(const rendering::RasterizationRenderer &renderer,const CParticleSystemComponent &ps,CParticleSystemComponent::OrientationType orientationType,bool bloom);
		bool BeginDraw(
			const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,
			CParticleSystemComponent &pSys,Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		);

		void GetParticleSystemOrientationInfo(
			const Mat4 &matrix,const CParticleSystemComponent &particle,CParticleSystemComponent::OrientationType orientationType,Vector3 &up,Vector3 &right,
			float &nearZ,float &farZ,const Material *material=nullptr,float camNearZ=0.f,float camFarZ=0.f
		) const;

		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
	protected:
		virtual prosper::DescriptorSetInfo &GetAnimationDescriptorSetInfo() const override;
		bool BindParticleMaterial(const rendering::RasterizationRenderer &renderer,const CParticleSystemComponent &ps);

		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetLightDescriptorSetIndex() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual bool ShouldInitializePipeline(uint32_t pipelineIdx) override;

		void RegisterDefaultGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		void RegisterDefaultGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		void RegisterDefaultGfxPipelineDescriptorSetGroups(prosper::GraphicsPipelineCreateInfo &pipelineInfo);

		void GetParticleSystemOrientationInfo(
			const Mat4 &matrix,const CParticleSystemComponent &particle,CParticleSystemComponent::OrientationType orientationType,Vector3 &up,Vector3 &right,
			float &nearZ,float &farZ,const Material *material=nullptr,const pragma::CCameraComponent *cam=nullptr
		) const;
		virtual void GetParticleSystemOrientationInfo(
			const Mat4 &matrix,const CParticleSystemComponent &particle,Vector3 &up,Vector3 &right,
			float &nearZ,float &farZ,const Material *material=nullptr,const pragma::CCameraComponent *cam=nullptr
		) const;
	};
};

#endif

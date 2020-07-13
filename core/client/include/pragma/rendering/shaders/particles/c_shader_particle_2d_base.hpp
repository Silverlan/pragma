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
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_PARTICLE;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_RADIUS;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_PREVPOS;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_AGE;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ROTATION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_LENGTH_YAW;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ANIMATION_START;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ANIMATION_FRAME_INDICES;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ANIMATION_INTERP_FACTOR;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_MAP;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_ANIMATION;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CSM;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;

		static constexpr auto VERTEX_COUNT = 6u;
		static constexpr auto TRIANGLE_COUNT = 2u;
		static std::array<Vector2,4> GetQuadVertexPositions();
		static Vector2 GetVertexUV(uint32_t vertIdx);
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

		ShaderParticle2DBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		bool Draw(const rendering::RasterizationRenderer &renderer,const CParticleSystemComponent &ps,CParticleSystemComponent::OrientationType orientationType,ParticleRenderFlags renderFlags);
		bool BeginDraw(
			const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,
			CParticleSystemComponent &pSys,ParticleRenderFlags renderFlags,Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		);

		void GetParticleSystemOrientationInfo(
			const Mat4 &matrix,const CParticleSystemComponent &particle,CParticleSystemComponent::OrientationType orientationType,Vector3 &up,Vector3 &right,
			float &nearZ,float &farZ,const Material *material=nullptr,float camNearZ=0.f,float camFarZ=0.f
		) const;

		Vector3 CalcVertexPosition(
			const pragma::CParticleSystemComponent &ptc,uint32_t ptIdx,uint32_t absVertIdx,
			const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
		) const;

		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
	protected:
		virtual Vector3 DoCalcVertexPosition(
			const pragma::CParticleSystemComponent &ptc,uint32_t ptIdx,uint32_t localVertIdx,
			const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
		) const;

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

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_PREPASS_HPP__
#define __C_SHADER_PREPASS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderPrepassBase
		: public ShaderEntity
	{
	public:
		enum class Pipeline : uint32_t
		{
			Regular = umath::to_integral(ShaderEntity::Pipeline::Regular),
			MultiSample = umath::to_integral(ShaderEntity::Pipeline::MultiSample),

			Reflection = umath::to_integral(ShaderEntity::Pipeline::Count),
			Count
		};
		enum class Flags : uint32_t
		{
			None = 0u,
			UseExtendedVertexWeights = 1u,
			RenderAs3DSky = UseExtendedVertexWeights<<1u
		};
		static Pipeline GetPipelineIndex(prosper::SampleCountFlags sampleCount);
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT_EXT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;

#pragma pack(push,1)
		struct PushConstants
		{
			Vector4 clipPlane;
			Vector4 drawOrigin; // w is scale
			uint32_t vertexAnimInfo;
			Flags flags;
		};
#pragma pack(pop)

		ShaderPrepassBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		ShaderPrepassBase(prosper::IPrContext &context,const std::string &identifier);

		bool BeginDraw(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,Pipeline pipelineIdx=Pipeline::Regular);
		bool BindClipPlane(const Vector4 &clipPlane);
		bool BindDrawOrigin(const Vector4 &drawOrigin);
		void Set3DSky(bool is3dSky);
		virtual bool Draw(CModelSubMesh &mesh) override;
	protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;

		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
	private:
		// These are unused
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
		virtual uint32_t GetLightDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
		virtual bool BindRenderSettings(prosper::IDescriptorSet &descSetRenderSettings) override {return false;}
		virtual bool BindLights(prosper::IDescriptorSet &dsLights) override {return false;}
		Flags m_stateFlags = Flags::None;
	};
	using ShaderPrepassDepth = ShaderPrepassBase;

	//////////////////

	class DLLCLIENT ShaderPrepass
		: public ShaderPrepassBase
	{
	public:
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_NORMAL;

		static prosper::Format RENDER_PASS_NORMAL_FORMAT;

		ShaderPrepass(prosper::IPrContext &context,const std::string &identifier);
	protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderPrepassBase::Flags)

#endif
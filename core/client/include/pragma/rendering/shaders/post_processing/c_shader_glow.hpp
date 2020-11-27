/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_GLOW_HPP__
#define __C_SHADER_GLOW_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderGlow
		: public ShaderTextured3DBase
	{
	public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

		static prosper::Format RENDER_PASS_FORMAT;

#pragma pack(push,1)
		struct PushConstants
		{
			float glowScale;
		};
#pragma pack(pop)

		ShaderGlow(prosper::IPrContext &context,const std::string &identifier);
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		bool BeginDraw(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,Pipeline pipelineIdx=Pipeline::Regular);
		bool BindGlowMaterial(CMaterial &mat);
	protected:
		virtual bool BindClipPlane(const Vector4 &clipPlane) override;
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
	};
};

#endif
// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_LIGHT_CONE_HPP__
#define __C_SHADER_LIGHT_CONE_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT ShaderLightCone : public ShaderGameWorldLightingPass {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_MAP;

#pragma pack(push, 1)
		struct PushConstants {
			float coneLength;
			uint32_t boundLightIndex;
			uint32_t resolution;
		};
#pragma pack(pop)

		ShaderLightCone(prosper::IPrContext &context, const std::string &identifier);
		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
	  protected:
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		int32_t m_boundLightIndex = -1;
	};
};

#endif
